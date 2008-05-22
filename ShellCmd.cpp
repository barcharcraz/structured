#include "ShellCmd.h"

void ShellCmd::pos_dice(vector<Cell_Data>cells,float eps){
FILE *	conf_ply_file=fopen("./dicepos.sh","w+"); 
FILE *		vripcmds_fp=fopen("mesh-pos/poscmds","w");
	FILE *	diced_fp=fopen("mesh-pos/diced.txt","w");

	for(int i=0; i <(int)cells.size(); i++){
	  if(cells[i].poses.size() == 0)
	    continue;
	  char conf_name[255];
	  char vrip_seg_fname[255];

	  sprintf(vrip_seg_fname,"mesh-pos/vripseg-%08d.txt",i);
	  sprintf(conf_name,"mesh-pos/bbox-clipped-diced-%08d.ply.txt",i);
	 FILE * vrip_seg_fp=fopen(vrip_seg_fname,"w");
	  FILE *bboxfp = fopen(conf_name,"w");
	  if(!vrip_seg_fp || !bboxfp){
	    printf("Unable to open %s\n",vrip_seg_fname);
	  }	
	  fprintf(diced_fp,"clipped-diced-%08d.ply\n",i);
	  fprintf(vripcmds_fp,"set BASEDIR=\"%s\"; set OUTDIR=\"mesh-pos/\";set VRIP_HOME=\"$BASEDIR/vrip\";setenv VRIP_DIR \"$VRIP_HOME/src/vrip/\";set path = ($path $VRIP_HOME/bin);cd %s/$OUTDIR;",basepath,cwd);
	  
	  for(int j=0; j <3; j++)
	    fprintf(vripcmds_fp,"plycullmaxx %f %f %f %f %f %f %f < ../mesh-pos/pos_rec-lod%d.ply > ../mesh-pos/clipped-diced-%08d-lod%d.ply;",//#set VISLIST=`cat ../%s | grep surface |cut -f1 -d\" \"`; plyclipbboxes -e %f $VISLIST ../mesh-agg/clipped-mb-%08d.ply > ../mesh-agg/vis-mb-%08d.ply;plyclipbboxes -e %f $VISLIST ../mesh-agg/clipped-mb-%08d.ply > ../mesh-agg/sub-mb-%08d.ply;",

		  cells[i].bounds.min_x,
		  cells[i].bounds.min_y,
		  FLT_MIN,
		  cells[i].bounds.max_x,
		  cells[i].bounds.max_y,
		  FLT_MAX,
		  eps,j,i,j);//vrip_seg_fname,2.0,i,i,1.0,i,i);
  
	  fprintf(vripcmds_fp,"\n");


	  for(unsigned int j=0; j <cells[i].poses.size(); j++){
	    const Stereo_Pose_Data *pose=cells[i].poses[j];
	    //Vrip List
	    fprintf(vrip_seg_fp,"%s 0.033 1\n",pose->mesh_name.c_str());
	    //Gen Tex File bbox
	    fprintf(bboxfp, "%d %s " ,pose->id,pose->left_name.c_str());
	    save_bbox_frame(pose->bbox,bboxfp);
	    for(int k=0; k < 4; k++)
	      for(int n=0; n < 4; n++)
		fprintf(bboxfp," %lf",pose->m[k][n]);
	    fprintf(bboxfp,"\n");
	  }
	  
	  if(have_mb_ply)
	    fprintf(vrip_seg_fp,"vis-mb-%08d.ply  0.1 0\n",i);
	  
	  fclose(vrip_seg_fp);
	  fclose(bboxfp);
	}
	if(have_mb_ply)
	  fprintf(diced_fp,"inv-mb.ply\n");

	fclose(diced_fp);
	fclose(vripcmds_fp);

	fprintf(conf_ply_file,"#!/bin/bash\nBASEPATH=%s/\nOUTDIR=$PWD/%s\nVRIP_HOME=%s/vrip\nexport VRIP_DIR=$VRIP_HOME/src/vrip/\nPATH=$PATH:$VRIP_HOME/bin:%s/tridecimator\ncd mesh-pos/\n",basepath,aggdir,basepath,basepath);
	fprintf(conf_ply_file,"%s/runtp.py poscmds\n",basepath);
	//	fprintf(conf_ply_file,"plysubtractlist pos_rec.ply ");
	//	for (int k=0; k < (int)cells.size(); k++)
	// fprintf(conf_ply_file," clipped-diced-%08d.ply",k);
		//	fprintf(conf_ply_file," > inv-mb.ply\n");

		fprintf(conf_ply_file,"for k in `echo {0..2}`\n"
			"do\n"
			"if [ -e clipped-diced-%08d-lod$k.ply ]; then\n"
		"\tplysubtract pos_rec-lod$k.ply clipped-diced-%08d-lod$k.ply > inv-mb-%08d-lod$k.ply\n"
		"else\n"
		"\tcp  pos_rec-lod$k.ply inv-mb-%08d-lod$k.ply\n"
		"fi\n",0,0,0,0); 

  fprintf(conf_ply_file,"for f in `echo {1..%ld}`\n"
		  "do\n"
		  "i=`printf \"%%08d\\n\" \"$f\"`\n"
		  "ilast=`printf \"%%08d\" \"$(($f - 1 ))\"`\n"
		  "if [ -e clipped-diced-$i-lod$k.ply ]; then\n"
		  "\tplysubtract inv-mb-$ilast-lod$k.ply clipped-diced-$i-lod$k.ply > inv-mb-$i-lod$k.ply;\n"
		  "else\n"
		  "\tcp inv-mb-$ilast-lod$k.ply  inv-mb-$i-lod$k.ply\n" 
		  "fi\n"
		  "done\n"
		  "tridecimator inv-mb-$i-lod$k.ply ../mesh-pos/inv-mb-lod$k.ply 0 \n"
	  // "tridecimator ../mesh-pos/inv-mb.ply ../mesh-pos/inv-mb-lod$f.ply 0 \n"
	  //"tridecimator ../mesh-pos/inv-mb.ply ../mesh-pos/inv-mb-lod1.ply 0\n"
	  //"tridecimator ../mesh-pos/inv-mb.ply ../mesh-pos/inv-mb-lod2.ply 0\n"
	
	/*	fprintf(conf_ply_file,"for f in `echo {0..2}`\n"
		"do\n"
		"\tVISLIST=`cat diced.txt | grep diced| sed s/.ply/-lod$f.ply/g`\n"
		"\tplymerge $VISLIST > merged-lod$f.ply\n"
		"\tplysubtract -i 0.1 pos_rec-lod$f.ply merged-lod$f.ply > inv-mb-lod$f.ply;\n"
	"done\n"	*/
	  "done\n"
		"cd ../mesh-pos/\n"
		"rm -f valid.txt\n"
	  "cat diced.txt |   while read MESHNAME; do\n"
	  "REALNAME=`echo $MESHNAME | sed s/.ply/-lod0.ply/g` \n"	 
	  "FACES=`plyhead $REALNAME | grep \"element face\" | cut -f 3 -d\" \"`\n"

	  "if [ $FACES == 0 ]; then\n continue;\n fi\n"
	  "echo $MESHNAME >> valid.txt\n"
	  "done\n"
	  "cat valid.txt |  sed s/.ply/-lod0.ply/g |xargs plybbox > range.txt\n"
	  ,cells.size()-1);






	fchmod(fileno(conf_ply_file),0777);
	fclose(conf_ply_file);
	system("./dicepos.sh");
}

void ShellCmd::pos_simp_cmd2(bool run){
  
  FILE *dicefp=fopen("./pos_simp.sh","w+");
  fprintf(dicefp,"#!/bin/bash\necho -e 'Simplifying...\\n'\nBASEPATH=%s/\nVRIP_HOME=$BASEPATH/vrip\nMESHAGG=$PWD/mesh-agg/\nexport VRIP_DIR=$VRIP_HOME/src/vrip/\nPATH=$PATH:$VRIP_HOME/bin\nRUNDIR=$PWD\nDICEDIR=$PWD/mesh-pos/\nmkdir -p $DICEDIR\ncd $MESHAGG\n",basepath);
  fprintf(dicefp,"cd $DICEDIR\n");
  fprintf(dicefp,"NUMDICED=`wc -l diced.txt |cut -f1 -d\" \" `\n"  
	  "REDFACT=(0.01 %f %f)\n",0.1,.40);
  if(dist_run){
  
    fprintf(dicefp, "LOGDIR=%s\n"
	    "if [[ -d $LOGDIR ]] ; then\n"
	    "find $LOGDIR -name 'loadbal*' | xargs rm &>/dev/null\nfi\n"
	    "if [[ -d $DICEDIR ]] ; then\n"
	    "find $DICEDIR -name '*lod*' | xargs rm &> /dev/null\nfi\n",pos_simp_log_dir);
  }
    fprintf(dicefp, 
	    "rm -f simpcmds\n"
	    "rm -f valid.txt\n"
	  "cat diced.txt | while read MESHNAME; do\n"
	  "FACES=`plyhead $MESHNAME | grep \"element face\" | cut -f 3 -d\" \"`\n"
	    "if [ $FACES == 0 ]; then\n continue;\n fi\n"
	    "echo $MESHNAME >> valid.txt\n"
	    "SIMPCMD=\"cd $DICEDIR/\" \n"
	    "\tfor k in `echo {0..2}`\n"
	    "\tdo\n"
	    "\t\tif [ $f == 0 ]; then\n"
	    "\t\t\tNEWNAME=`echo $MESHNAME | sed s/.ply/-lod$f.ply/g`\n"
	  "FLIPCMD=\"-F\"\n"
	  "\t\telse\n"
	  "FLIPCMD="
	  "\t\t\tNEWNAME=`echo $MESHNAME | sed s/-lod$(($f - 1 )).ply/-lod$f.ply/g`\n"
	  "\t\tfi\n"
	  "\t\tSIMPCMD=$SIMPCMD\";\"\"$BASEPATH/tridecimator/tridecimator $MESHNAME $NEWNAME ${REDFACT[$f]}r -b2.0 $FLIPCMD >& declog-$MESHNAME.txt ;chmod 0666 $NEWNAME  \"\n"
	  "MESHNAME=$NEWNAME\n"
	  "\tdone\n"
	  "echo $SIMPCMD >> simpcmds\n"
	  "done\n");
  
  
  if(dist_run){
    fprintf(dicefp,"cd $DICEDIR\n"
	    "time $BASEPATH/vrip/bin/loadbalance ~/loadlimit simpcmds -logdir $LOGDIR\n");
  } else {
    fprintf(dicefp,"time %s/runtp.py simpcmds\n",basepath);
  }

  fprintf(dicefp,"cat valid.txt | xargs plybbox > range.txt\n");
  fchmod(fileno(dicefp),0777);
  fclose(dicefp);
  if(run)
    system("./pos_simp.sh");
  
}
void ShellCmd::pos_simp_cmd(bool run){
  
  FILE *dicefp=fopen("./pos_simp.sh","w+");
  fprintf(dicefp,"#!/bin/bash\necho -e 'Simplifying...\\n'\nBASEPATH=%s/\nVRIP_HOME=$BASEPATH/vrip\nMESHAGG=$PWD/mesh-agg/\nexport VRIP_DIR=$VRIP_HOME/src/vrip/\nPATH=$PATH:$VRIP_HOME/bin\nRUNDIR=$PWD\nDICEDIR=$PWD/mesh-pos/\nmkdir -p $DICEDIR\ncd $MESHAGG\n",basepath);
  fprintf(dicefp,"cd $DICEDIR\n");
  fprintf(dicefp,"NUMDICED=`wc -l diced.txt |cut -f1 -d\" \" `\n"  
	  "REDFACT=(0.01 %f %f)\n",0.1,.40);
  if(dist_run){
  
    fprintf(dicefp, "LOGDIR=%s\n"
	    "if [[ -d $LOGDIR ]] ; then\n"
	    "find $LOGDIR -name 'loadbal*' | xargs rm &>/dev/null\nfi\n"
	    "if [[ -d $DICEDIR ]] ; then\n"
	    "find $DICEDIR -name '*lod*' | xargs rm &> /dev/null\nfi\n",
	    pos_simp_log_dir);
  }
    fprintf(dicefp, 
	    "rm -f simpcmds\n"
	    "rm -f valid.txt\n"
	  "cat diced.txt | while read MESHNAME; do\n"
	  "FACES=`plyhead $MESHNAME | grep \"element face\" | cut -f 3 -d\" \"`\n"
	    "if [ $FACES == 0 ]; then\n continue;\n fi\n"
	    "echo $MESHNAME >> valid.txt\n"
	    "SIMPCMD=\"cd $DICEDIR/\" \n"
	    "\tfor f in `echo {0..2}`\n"
	    "\tdo\n"
	    "\t\tif [ $f == 0 ]; then\n"
	    "\t\t\tNEWNAME=`echo $MESHNAME | sed s/.ply/-lod$f.ply/g`\n"
	  "FLIPCMD=\"-F\"\n"
	  "\t\telse\n"
	  "FLIPCMD="
	  "\t\t\tNEWNAME=`echo $MESHNAME | sed s/-lod$(($f - 1 )).ply/-lod$f.ply/g`\n"
	  "\t\tfi\n"
	  "\t\tSIMPCMD=$SIMPCMD\";\"\"$BASEPATH/tridecimator/tridecimator $MESHNAME $NEWNAME ${REDFACT[$f]}r -b2.0 $FLIPCMD >& declog-$MESHNAME.txt ;chmod 0666 $NEWNAME  \"\n"
	  "MESHNAME=$NEWNAME\n"
	  "\tdone\n"
	  "echo $SIMPCMD >> simpcmds\n"
	  "done\n");
  
  
  if(dist_run){
    fprintf(dicefp,"cd $DICEDIR\n"
	    "time $BASEPATH/vrip/bin/loadbalance ~/loadlimit simpcmds -logdir $LOGDIR\n");
  } else {
    fprintf(dicefp,"time %s/runtp.py simpcmds\n",basepath);
  }

  fprintf(dicefp,"cat valid.txt | xargs plybbox > range.txt\n");
  fchmod(fileno(dicefp),0777);
  fclose(dicefp);
  if(run)
    system("./pos_simp.sh");
  
}
