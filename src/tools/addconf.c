/*
 * $Id$
 * 
 *       This source code is part of
 * 
 *        G   R   O   M   A   C   S
 * 
 * GROningen MAchine for Chemical Simulations
 * 
 *               VERSION 2.0
 * 
 * Copyright (c) 1991-1999
 * BIOSON Research Institute, Dept. of Biophysical Chemistry
 * University of Groningen, The Netherlands
 * 
 * Please refer to:
 * GROMACS: A message-passing parallel molecular dynamics implementation
 * H.J.C. Berendsen, D. van der Spoel and R. van Drunen
 * Comp. Phys. Comm. 91, 43-56 (1995)
 * 
 * Also check out our WWW page:
 * http://md.chem.rug.nl/~gmx
 * or e-mail to:
 * gromacs@chem.rug.nl
 * 
 * And Hey:
 * GRowing Old MAkes el Chrono Sweat
 */
static char *SRCID_addconf_c = "$Id$";

#include <stdlib.h>
#include <string.h>
#include "vec.h"
#include "assert.h"
#include "macros.h"
#include "smalloc.h"
#include "addconf.h"
#include "force.h"
#include "gstat.h"
#include "princ.h"
#include "rdgroup.h"
#include "txtdump.h"
#include "pbc.h"
#include "names.h"
#include "nsgrid.h"
#include "mdatoms.h"
#include "nrnb.h"
#include "ns.h"
#include "../mdlib/wnblist.h"

static real box_margin;

static real max_dist(rvec *x, real *r, int start, int end)
{
  real maxd;
  int i,j;
  
  maxd=0;
  for(i=start; i<end; i++)
    for(j=i+1; j<end; j++)
      maxd=max(maxd,sqrt(distance2(x[i],x[j]))+0.5*(r[i]+r[j]));
  
  return 0.5*maxd;
}

static void set_margin(t_atoms *atoms, rvec *x, real *r)
{
  int i,d,start;
  
  box_margin=0;
  
  start=0;
  for(i=0; i < atoms->nr; i++) {
    if ( (i+1 == atoms->nr) || 
	 (atoms->atom[i+1].resnr != atoms->atom[i].resnr) ) {
      d=max_dist(x,r,start,i+1);
      if (debug && d>box_margin)
	fprintf(debug,"getting margin from %s: %g\n",
		*(atoms->resname[atoms->atom[i].resnr]),box_margin);
      box_margin=max(box_margin,d);
      start=i+1;
    }
  }
  fprintf(stderr,"box_margin = %g\n",box_margin);
}

static bool outside_box_minus_margin2(rvec x,matrix box)
{
  return ( (x[XX]<2*box_margin) || (x[XX]>box[XX][XX]-2*box_margin) ||
	   (x[YY]<2*box_margin) || (x[YY]>box[YY][YY]-2*box_margin) ||
	   (x[ZZ]<2*box_margin) || (x[ZZ]>box[ZZ][ZZ]-2*box_margin) );
}

static bool outside_box_plus_margin(rvec x,matrix box)
{
  return ( (x[XX]<-box_margin) || (x[XX]>box[XX][XX]+box_margin) ||
	   (x[YY]<-box_margin) || (x[YY]>box[YY][YY]+box_margin) ||
	   (x[ZZ]<-box_margin) || (x[ZZ]>box[ZZ][ZZ]+box_margin) );
}

static int mark_res(int at, bool *mark, int natoms, t_atom *atom,int *nmark)
{
  int resnr;
  
  resnr = atom[at].resnr;
  while( (at > 0) && (resnr==atom[at-1].resnr) )
    at--;
  while( (at < natoms) && (resnr==atom[at].resnr) ) {
    if (!mark[at]) {
      mark[at]=TRUE;
      (*nmark)++;
    }
    at++;
  }
  
  return at;
}

static real find_max_real(int n,real radius[])
{
  int  i;
  real rmax;
  
  rmax = 0;
  if (n > 0) {
    rmax = radius[0];
    for(i=1; (i<n); i++)
      rmax = max(rmax,radius[i]);
  }
  return rmax;
}

static void combine_atoms(t_atoms *ap,t_atoms *as,rvec xp[],rvec xs[],
			  t_atoms **a_comb,rvec **x_comb)
{
  t_atoms *ac;
  rvec    *xc;
  int     i,j,natot,res0;
  
  /* Total number of atoms */
  natot = ap->nr+as->nr;
  
  snew(ac,1);
  init_t_atoms(ac,natot,FALSE);
  stupid_fill(&(ac->excl),natot,FALSE);

  snew(xc,natot);
    
  /* Fill the new structures */
  for(i=j=0; (i<ap->nr); i++,j++) {
    copy_rvec(xp[i],xc[j]);
    memcpy(&(ac->atom[j]),&(ap->atom[i]),sizeof(ap->atom[i]));
    ac->atom[j].type = 0;
  }
  res0 = ap->nres;
  for(i=0; (i<as->nr); i++,j++) {
    copy_rvec(xs[i],xc[j]);
    memcpy(&(ac->atom[j]),&(as->atom[i]),sizeof(as->atom[i]));
    ac->atom[j].type   = 0;
    ac->atom[j].resnr += res0;
  }
  ac->nr   = j;
  ac->nres = ac->atom[j-1].resnr+1;
    
  /* Return values */
  *a_comb = ac;
  *x_comb = xc;
}

static t_forcerec *fr=NULL;

void do_nsgrid(FILE *fp,bool bVerbose,
	       matrix box,rvec x[],t_atoms *atoms,real rlong)
{
  static bool bFirst = TRUE;
  static t_topology *top;
  static t_nsborder *nsb;
  static t_mdatoms  *md;
  static t_block    *cgs;
  static t_inputrec *ir;
  static t_nrnb     nrnb;
  static t_commrec  *cr;
  static t_groups   *grps;
  static int        *cg_index;

  int        i,j,m,natoms,nx,ny,nz,res0;
  ivec       *nFreeze;
  rvec       box_size;
  real       lambda=0,dvdlambda=0;

  natoms = atoms->nr;
    
  if (bFirst) {
    /* Charge group index */  
    snew(cg_index,natoms);
    for(i=0; (i<natoms); i++)
      cg_index[i]=i;
    
    /* Topology needs charge groups and exclusions */
    snew(top,1);
    init_top(top);
    stupid_fill(&(top->blocks[ebCGS]),natoms,FALSE);
    memcpy(&(top->atoms),atoms,sizeof(*atoms));
    top->atoms.grps[egcENER].nr = 1;
    
    /* Some nasty shortcuts */
    cgs  = &(top->blocks[ebCGS]);
    
    top->idef.ntypes = 1;
    top->idef.pid    = 0;
    top->idef.atnr   = 1;
    snew(top->idef.functype,1);
    snew(top->idef.iparams,1);
    top->idef.iparams[0].lj.c6  = 1;
    top->idef.iparams[0].lj.c12 = 1;

    /* mdatoms structure */
    snew(nFreeze,2);
    md = atoms2md(debug,atoms,nFreeze,FALSE,FALSE,FALSE);
    sfree(nFreeze);

    /* nsborder struct */
    snew(nsb,1);
    nsb->pid    = 0;
    nsb->nprocs = 1;
    calc_nsb(debug,&(top->blocks[ebCGS]),1,nsb,0);
    if (debug)
      print_nsb(debug,"nsborder",nsb);
  
    /* inputrec structure */
    snew(ir,1);
    ir->coulombtype = eelCUT;
    ir->vdwtype     = evdwCUT;
    ir->ndelta      = 2;
    ir->ns_type     = ensGRID;
    ir->solvent_opt = -1;
    snew(ir->opts.eg_excl,1);
    
    /* forcerec structure */
    if (fr == NULL)
      fr = mk_forcerec();
    snew(cr,1);
    cr->nprocs = 1;
    
    ir->rlist       = ir->rcoulomb = ir->rvdw = rlong;
    init_forcerec(debug,fr,ir,&(top->blocks[ebMOLS]),cr,
		  &(top->blocks[ebCGS]),&(top->idef),md,nsb,box,FALSE);
    fr->cg0 = 0;
    fr->hcg = top->blocks[ebCGS].nr;
    fr->nWatMol = 0;
    if (debug)
      pr_forcerec(debug,fr,cr);
    
    /* Prepare for neighboursearching */
    init_nrnb(&nrnb);

    /* Group stuff */
    snew(grps,1);
    
    bFirst = FALSE;
  }

  /* Init things dependent on parameters */  
  ir->rlist       = ir->rcoulomb = ir->rvdw = rlong;
  init_forcerec(debug,fr,ir,&(top->blocks[ebMOLS]),cr,
		&(top->blocks[ebCGS]),&(top->idef),md,nsb,box,FALSE);
		
  /* Calculate new stuff dependent on coords and box */
  for(m=0; (m<DIM); m++)
    box_size[m] = box[m][m];
  calc_shifts(box,box_size,fr->shift_vec,FALSE);
  put_charge_groups_in_box(fp,0,cgs->nr,FALSE,box,box_size,cgs,
			   x,fr->cg_cm);
  
  /* Do the actual neighboursearching */
  init_neighbor_list(fp,fr,HOMENR(nsb));
  search_neighbours(fp,fr,x,box,top,grps,cr,nsb,&nrnb,md,lambda,&dvdlambda);

  if (debug)
    dump_nblist(debug,fr,0);

  if (bVerbose)    
    fprintf(stderr,"Succesfully made neighbourlist\n");
}

bool bXor(bool b1,bool b2)
{
  return (b1 && !b2) || (b2 && !b1);
}

void add_conf(t_atoms *atoms, rvec **x, real **r,  bool bSrenew,  matrix box,
	      t_atoms *atoms_solvt, rvec *x_solvt, real *r_solvt, 
	      bool bVerbose,real rshell)
{
  t_nblist   *nlist;
  t_atoms    *atoms_all;
  real       max_vdw,*r_prot,*r_all,n2,r2,ib1,ib2;
  int        natoms_prot,natoms_solvt;
  int        i,j,jj,m,j0,j1,jnr,inr,iprot,is1,is2;
  int        prev,resnr,nresadd,d,k,ncells,maxincell;
  int        dx0,dx1,dy0,dy1,dz0,dz1;
  int        *atom_flag,*cg_index;
  int        ntest,nremove,nkeep;
  rvec       dx,xi,xj,*x_prot,xpp,*x_all;
  bool       *remove,*keep;
  int        bSolSol;

  natoms_prot  = atoms->nr;
  natoms_solvt = atoms_solvt->nr;
  if (natoms_solvt <= 0) {
    fprintf(stderr,"WARNING: Nothing to add\n");
    return;
  }
  
  if (bVerbose)
    fprintf(stderr,"Calculating Overlap...\n");
  
  /* Set margin around box edges to largest solvent dimension.
   * The maximum distance between atoms in a solvent molecule should
   * be calculated. At the moment a fudge factor of 3 is used.
   */
  r_prot     = *r;
  box_margin = 3*find_max_real(natoms_solvt,r_solvt);
  max_vdw    = max(3*find_max_real(natoms_prot,r_prot),box_margin);
  fprintf(stderr,"box_margin = %g\n",box_margin);
  
  snew(remove,natoms_solvt);
  init_pbc(box,FALSE);

  nremove = 0;
  for(i=0; (i<atoms_solvt->nr); i++)
    if ( outside_box_plus_margin(x_solvt[i],box) )
      i=mark_res(i,remove,atoms_solvt->nr,atoms_solvt->atom,&nremove);
  fprintf(stderr,"Removed %d atoms that were outside the box\n",nremove);
  
  /* Define grid stuff for genbox */
  /* Largest VDW radius */
  snew(r_all,natoms_prot+natoms_solvt);
  for(i=j=0; (i<natoms_prot); i++,j++)
    r_all[j]=r_prot[i];
  for(i=0; (i<natoms_solvt); i++,j++)
    r_all[j]=r_solvt[i];

  /* Combine arrays */
  combine_atoms(atoms,atoms_solvt,*x,x_solvt,&atoms_all,&x_all);
	     
  /* Do neighboursearching step */
  do_nsgrid(stdout,bVerbose,box,x_all,atoms_all,max_vdw);
  
  /* check solvent with solute */
  nlist = &(fr->nlist_sr[eNL_VDW]);
  fprintf(stderr,"nri = %d, nrj = %d\n",nlist->nri,nlist->nrj);
  for(bSolSol=0; (bSolSol<=1); bSolSol++) {
    ntest = nremove = 0;
    fprintf(stderr,"Checking %s-Solvent overlap:",
	    bSolSol ? "Solvent" : "Protein");
    for(i=0; (i<nlist->nri); i++) {
      inr = nlist->iinr[i];
      j0  = nlist->jindex[i];
      j1  = nlist->jindex[i+1];
      rvec_add(x_all[inr],fr->shift_vec[nlist->shift[i]],xi);
      
      for(j=j0; (j<j1); j++) {
	jnr = nlist->jjnr[j];
	copy_rvec(x_all[jnr],xj);
	
	/* Check solvent-protein and solvent-solvent */
	is1 = inr-natoms_prot;
	is2 = jnr-natoms_prot;
	
	/* Check if at least one of the atoms is a solvent that is not yet
	 * listed for removal, and if both are solvent, that they are not in the
	 * same residue.
	 */
	if ((!bSolSol && 
	     bXor((is1 >= 0),(is2 >= 0)) &&  /* One atom is protein */
	     ((is1 < 0) || ((is1 >= 0) && !remove[is1])) &&
	     ((is2 < 0) || ((is2 >= 0) && !remove[is2]))) ||
	    
	    (bSolSol  && 
	     (is1 >= 0) && (!remove[is1]) &&   /* is1 is solvent */
	     (is2 >= 0) && (!remove[is2]) &&   /* is2 is solvent */
	     (outside_box_minus_margin2(x_solvt[is1],box)) && /* is1 on edge */
	     (outside_box_minus_margin2(x_solvt[is2],box)) && /* is2 on edge */
	     (atoms_solvt->atom[is1].resnr !=  /* Not the same residue */
	      atoms_solvt->atom[is2].resnr))) {
		
	  ntest++;
	  rvec_sub(xi,xj,dx);
	  n2 = norm2(dx);
	  r2 = sqr(r_all[inr]+r_all[jnr]);
	  if (n2 < r2) {
	    /* Need only remove one of the solvents... */
	    if (is2 >= 0)
	      (void) mark_res(is2,remove,natoms_solvt,atoms_solvt->atom,
			      &nremove);
	    else if (is1 >= 0)
	      (void) mark_res(is1,remove,natoms_solvt,atoms_solvt->atom,
			      &nremove);
	    else
	      fprintf(stderr,"Neither atom is solvent%d %d\n",is1,is2);
	  }
	}
      }
    }
    fprintf(stderr," tested %d pairs, removed %d atoms.\n",ntest,nremove);
  }
  if (debug) 
    for(i=0; (i<natoms_solvt); i++)
      fprintf(debug,"remove[%5d] = %s\n",i,bool_names[remove[i]]);
      
  /* Search again, now with another cut-off */
  if (rshell > 0) {
    do_nsgrid(stdout,bVerbose,box,x_all,atoms_all,rshell);
    nkeep = 0;
    snew(keep,natoms_solvt);
    for(i=0; (i<nlist->nri); i++) {
      inr = nlist->iinr[i];
      j0  = nlist->jindex[i];
      j1  = nlist->jindex[i+1];
      
      for(j=j0; (j<j1); j++) {
	jnr = nlist->jjnr[j];
	
	/* Check solvent-protein and solvent-solvent */
	is1 = inr-natoms_prot;
	is2 = jnr-natoms_prot;
	
	/* Check if at least one of the atoms is a solvent that is not yet
	 * listed for removal, and if both are solvent, that they are not in the
	 * same residue.
	 */
	if ((is1 >= 0) && (is2 < 0)) 
	  (void) mark_res(is1,keep,natoms_solvt,atoms_solvt->atom,&nkeep);
	else if ((is1 < 0) && (is2 >= 0)) 
	  (void) mark_res(is2,keep,natoms_solvt,atoms_solvt->atom,&nkeep);
      }
    }
    fprintf(stderr,"Keeping %d solvent atoms after proximity check\n",
	    nkeep);
    for (i=0; (i<natoms_solvt); i++)
      remove[i] = remove[i] || !keep[i];
    sfree(keep);
  }
  /* count how many atoms will be added and make space */
  j=0;
  for (i=0; (i<atoms_solvt->nr); i++)
    if (!remove[i])
      j++;
  if (bSrenew) {
    srenew(atoms->resname,  atoms->nres+atoms_solvt->nres);
    srenew(atoms->atomname, atoms->nr+j);
    srenew(atoms->atom,     atoms->nr+j);
    srenew(*x,              atoms->nr+j);
    srenew(*r,              atoms->nr+j);
  }
  
  /* add the selected atoms_solvt to atoms */
  prev=NOTSET;
  nresadd=0;
  x_prot = *x;
  for (i=0; (i<atoms_solvt->nr); i++)
    if (!remove[i]) {
      if ( (prev==NOTSET) || 
	   (atoms_solvt->atom[i].resnr != atoms_solvt->atom[prev].resnr) ) {
	nresadd ++;
	atoms->nres++;
	/* calculate shift of the solvent molecule using the first atom */
	copy_rvec(x_solvt[i],dx);
	put_atoms_in_box(box,1,&dx);
	rvec_dec(dx,x_solvt[i]);
      }
      atoms->nr++;
      atoms->atomname[atoms->nr-1] = atoms_solvt->atomname[i];
      rvec_add(x_solvt[i],dx,x_prot[atoms->nr-1]);
      (*r)[atoms->nr-1]   = r_solvt[i];
      atoms->atom[atoms->nr-1].resnr = atoms->nres-1;
      atoms->resname[atoms->nres-1] =
	atoms_solvt->resname[atoms_solvt->atom[i].resnr];
      prev=i;
    }
  if (bSrenew)
    srenew(atoms->resname,  atoms->nres+nresadd);
  
  if (bVerbose)
    fprintf(stderr,"Added %d molecules\n",nresadd);

  sfree(remove);
}

void orient_mol(t_atoms *atoms,char *indexnm,rvec x[], rvec *v)
{
  int     isize;
  atom_id *index,*simp;
  char    *grpnames;
  real    totmass;
  int     i,m;
  rvec    xcm,prcomp;
  matrix  trans;
  
  /* Make an index for principal component analysis */
  fprintf(stderr,"\nSelect group for orientation of molecule:\n");
  get_index(atoms,indexnm,1,&isize,&index,&grpnames);
  snew(simp,atoms->nr);
  for(i=0; (i<atoms->nr); i++) {
    simp[i]=i;
    atoms->atom[i].m=1;
  }
  totmass = sub_xcm(x,atoms->nr,simp,atoms->atom,xcm,FALSE);
  principal_comp(isize,index,atoms->atom,x,trans,prcomp);
  
  /* Check whether this trans matrix mirrors the molecule */
  if (det(trans) < 0) {
    if (debug)
      fprintf(stderr,"Mirroring rotation matrix in Z direction\n");
    for(m=0; (m<DIM); m++)
      trans[ZZ][m] = -trans[ZZ][m];
  }  
  rotate_atoms(atoms->nr,simp,x,trans);
  
  if (debug) {
    pr_rvecs(stderr,0,"Rot Matrix",trans,DIM);
    fprintf(stderr,"Det(trans) = %g\n",det(trans));
    
    /* print principal component data */
    fprintf(stderr,"Norm of principal axes before rotation: "
	    "(%.3f, %.3f, %.3f)\n",prcomp[XX],prcomp[YY],prcomp[ZZ]);
    fprintf(stderr,"Totmass = %g\n",totmass);
    principal_comp(isize,index,atoms->atom,x,trans,prcomp);
    rotate_atoms(atoms->nr,simp,x,trans);
    if (v) 
      rotate_atoms(atoms->nr,simp,v,trans);
    pr_rvecs(stderr,0,"Rot Matrix",trans,DIM);
  }
  sfree(simp);
  sfree(index);
  sfree(grpnames);
}

