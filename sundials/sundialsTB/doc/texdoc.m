function [] = texdoc
% TEXDOC - Creates LaTeX documentation for sundialsTB
%

% Radu Serban <radu@llnl.gov>
% LLNS Copyright Start
% Copyright (c) 2014, Lawrence Livermore National Security
% This work was performed under the auspices of the U.S. Department 
% of Energy by Lawrence Livermore National Laboratory in part under 
% Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
% Produced at the Lawrence Livermore National Laboratory.
% All rights reserved.
% For details, see the LICENSE file.
% LLNS Copyright End
% $Revision: 4075 $Date: 2009/04/22 04:14:36 $


global cv_fct cv_ftp cv_es cv_ep
global ida_fct ida_ftp ida_es
global kin_fct kin_ftp kin_es kin_ep
global nvec_fct putils_fct

% If the output directory does not exist, create it
system('mkdir -p tex');

% Set output dir
cd('..');
doc_dir = 'doc/tex';

% Set list of files to be processed
set_files;

%==================================================================      

file = sprintf('%s/sundialsTB.tex',doc_dir);
fid = fopen(file,'w');

fprintf(fid,'\\input{front}\n\n');

%==================================================================      

fprintf(fid,'\\input{cvodes_top}\n\n');

% CVODES interface functions

fprintf(fid,'\\newpage\n\\subsection{Interface functions}\n');
fprintf(fid,'\\rule{0in}{0.25in}\n\n');
for i = 1:length(cv_fct)
  process_no_source(cv_fct{i}, doc_dir, fid);
end

% CVODES function types

fprintf(fid,'\n\\newpage\n\\subsection{Function types}\n'); 
fprintf(fid,'\\rule{0in}{0.25in}\n\n');
for i = 1:length(cv_ftp)
  process_no_source(cv_ftp{i}, doc_dir, fid);
end

%==================================================================      

fprintf(fid,'\\input{idas_top}\n\n');

% IDAS interface functions

fprintf(fid,'\\newpage\n\\subsection{Interface functions}\n');
fprintf(fid,'\\rule{0in}{0.25in}\n\n');
for i = 1:length(ida_fct)
  process_no_source(ida_fct{i}, doc_dir, fid);
end

% IDAS function types

fprintf(fid,'\n\\newpage\n\\subsection{Function types}\n'); 
fprintf(fid,'\\rule{0in}{0.25in}\n\n');
for i = 1:length(ida_ftp)
  process_no_source(ida_ftp{i}, doc_dir, fid);
end

%==================================================================

fprintf(fid,'\\input{kinsol_top}\n\n');

% KINSOL interface functions

fprintf(fid,'\\newpage\n\\subsection{Interface functions}\n');
fprintf(fid,'\\rule{0in}{0.25in}\n\n');
for i = 1:length(kin_fct)
  process_no_source(kin_fct{i}, doc_dir, fid);
end

% KINSOL function types

fprintf(fid,'\n\\newpage\n\\subsection{Function types}\n'); 
fprintf(fid,'\\rule{0in}{0.25in}\n\n');
for i = 1:length(kin_ftp)
  process_no_source(kin_ftp{i}, doc_dir, fid);
end

%==================================================================

fprintf(fid,'\n\n\\input{other_top}\n\n');

fprintf(fid,'\n\n\\newpage\n\\subsection{{\\nvector} functions}\n');
fprintf(fid,'\\rule{0in}{0.25in}\n\n');
for i = 1:length(nvec_fct)
  process_with_source(nvec_fct{i}, doc_dir, fid);
end

fprintf(fid,'\n\n\\newpage\n\\subsection{Parallel utilities}\n');
fprintf(fid,'\\rule{0in}{0.25in}\n\n');
for i = 1:length(putils_fct)
  process_no_source(putils_fct{i}, doc_dir, fid);
end

%==================================================================   

fprintf(fid,'\n\n\\newpage\n\\appendix\n\n');

fprintf(fid,'\n\n\\section{Implementation of CVodeMonitor.m}\n');
process_with_source('cvodes/CVodeMonitor.m', doc_dir, fid);
process_no_source('cvodes/CVodeMonitorB.m', doc_dir, fid);

fprintf(fid,'\n\n\\section{Implementation of IDAMonitor.m}\n');
process_with_source('idas/IDAMonitor.m', doc_dir, fid);
process_no_source('idas/IDAMonitorB.m', doc_dir, fid);


%==================================================================   

fprintf(fid,'\n\n\\newpage\n\\include{references}\n\\printindex\n\\end{document}');

fclose(fid);



system('cp doc/tex_files/*.tex doc/tex/');
system('cp doc/tex_files/biblio.bib doc/tex/');
system('cp doc/tex_files/doc_logo.eps doc/tex/');


cd(doc_dir);

fprintf('Compile LaTeX files...\n');
system('latex sundialsTB');
system('bibtex sundialsTB');
system('makeindex sundialsTB');

system('sed ''s$_$\\_$'' sundialsTB.ind > foo.ind');
system('mv foo.ind sundialsTB.ind');

system('latex sundialsTB');
system('latex sundialsTB');

fprintf('Generate PS file...\n');
system('dvips -Ppdf -o sundialsTB.ps sundialsTB');

fprintf('Generate PDF file...\n');
system('dvipdf sundialsTB');

system('cp sundialsTB.ps ..');
system('cp sundialsTB.pdf ..');

cd('..');

%==================================================================   
% Process matlab file. Do not generate source
%==================================================================   

function process_no_source(name, doc_dir, fid)

m2html('mfiles',name,...
       'verbose','off',...
       'syntaxHighlighting','off',...
       'source','off',...
       'htmldir',doc_dir,...
       'extension','.tex',...
       'indexFile','foo',...
       'template','latex');

fprintf('%s\n',name);

ii = strfind(name,'.m');     
fprintf(fid,'\\input{%s}\n',name(1:ii-1)); 


%==================================================================   
% Process matlab file. Generated highlighted source code.
%==================================================================   

function process_with_source(name, doc_dir, fid)

m2html('mfiles',name,...
       'verbose','off',...
       'syntaxHighlighting','off',...
       'htmldir',doc_dir,...
       'extension','.tex',...
       'source','on', ...
       'indexFile','foo',...
       'template','latex');
  
ii = strfind(name,'.m');
fi = fopen(name,'r');
fo = fopen(sprintf('%s/%ssrc.tex',doc_dir, name(1:ii-1)),'w');

start = 0;
finish = 0;

l = fgets(fi);
if l(1) == '%'
  start = 1;
end
i = 2;
while ~feof(fi)
  l = fgets(fi);
  if l(1) ~= '%'
    if finish == 0, finish = i-1; end
  else
    if start == 0,  start = i;    end
  end  
  i = i + 1;
end
last = i;
frewind(fi);

fprintf('%s   %d  %d  %d\n',name,start,finish,i);


if start == 1
  lines = sprintf('linerange={%d-%d}',finish+1,last);
else
  lines = sprintf('linerange={1-%d,%d-%d}',start-1,finish+1,last);  
end

fprintf(fo,'\\begin{lstlisting}[%s]\n',lines);
for i = 1: last-1
  fprintf(fo,'%s',fgets(fi));
end
fprintf(fo,'\\end{lstlisting}\n');

fclose(fo);
fclose(fi);

fprintf(fid,'\\input{%s}\n',name(1:ii-1)); 

%==================================================================   
% Set files
%==================================================================   

function [] = set_files()

global cv_fct cv_ftp cv_es cv_ep
global ida_fct ida_ftp ida_es
global kin_fct kin_ftp kin_es kin_ep
global nvec_fct putils_fct

cv_fct = {
    'cvodes/CVodeSetOptions.m'...
    'cvodes/CVodeQuadSetOptions.m'...
    'cvodes/CVodeSensSetOptions.m'...
    'cvodes/CVodeInit.m'...
    'cvodes/CVodeQuadInit.m'...
    'cvodes/CVodeSensInit.m'...
    'cvodes/CVodeAdjInit.m'...
    'cvodes/CVodeInitB.m'...
    'cvodes/CVodeQuadInitB.m'...
    'cvodes/CVodeReInit.m'...
    'cvodes/CVodeQuadReInit.m'...
    'cvodes/CVodeSensReInit.m'...
    'cvodes/CVodeAdjReInit.m'...
    'cvodes/CVodeReInitB.m'...
    'cvodes/CVodeQuadReInitB.m'...
    'cvodes/CVode.m'...
    'cvodes/CVodeB.m'...
    'cvodes/CVodeSensToggleOff.m'...
    'cvodes/CVodeGetStats.m'...
    'cvodes/CVodeGetStatsB.m'...
    'cvodes/CVodeGet.m'...
    'cvodes/CVodeSet.m'...
    'cvodes/CVodeSetB.m'...
    'cvodes/CVodeFree.m'...
         };

cv_ftp = {
    'cvodes/function_types/CVRhsFn.m'...
    'cvodes/function_types/CVSensRhsFn.m'...
    'cvodes/function_types/CVQuadRhsFn.m'...
    'cvodes/function_types/CVRootFn.m'...
    'cvodes/function_types/CVDenseJacFn.m'...
    'cvodes/function_types/CVBandJacFn.m'...
    'cvodes/function_types/CVJacTimesVecFn.m'...
    'cvodes/function_types/CVPrecSetupFn.m'...
    'cvodes/function_types/CVPrecSolveFn.m'...
    'cvodes/function_types/CVGcommFn.m'...
    'cvodes/function_types/CVGlocalFn.m'...
    'cvodes/function_types/CVMonitorFn.m'...
    'cvodes/function_types/CVRhsFnB.m'...
    'cvodes/function_types/CVQuadRhsFnB.m'...
    'cvodes/function_types/CVDenseJacFnB.m'...
    'cvodes/function_types/CVBandJacFnB.m'...
    'cvodes/function_types/CVJacTimesVecFnB.m'...
    'cvodes/function_types/CVPrecSetupFnB.m'...
    'cvodes/function_types/CVPrecSolveFnB.m'...
    'cvodes/function_types/CVGcommFnB.m'...
    'cvodes/function_types/CVGlocalFnB.m'...
    'cvodes/function_types/CVMonitorFnB.m'...
         };

ida_fct = {
    'idas/IDASetOptions.m'...
    'idas/IDAQuadSetOptions.m'...
    'idas/IDASensSetOptions.m'...
    'idas/IDAInit.m'...
    'idas/IDAQuadInit.m'...
    'idas/IDASensInit.m'...
    'idas/IDAAdjInit.m'...
    'idas/IDAInitB.m'...
    'idas/IDAQuadInitB.m'...
    'idas/IDAReInit.m'...
    'idas/IDAQuadReInit.m'...
    'idas/IDASensReInit.m'...
    'idas/IDAAdjReInit.m'...
    'idas/IDAReInitB.m'...
    'idas/IDAQuadReInitB.m'...
    'idas/IDACalcIC.m'...
    'idas/IDACalcICB.m'...
    'idas/IDASolve.m'...
    'idas/IDASolveB.m'...
    'idas/IDASensToggleOff.m'...
    'idas/IDAGetStats.m'...
    'idas/IDAGetStatsB.m'...
    'idas/IDAGet.m'...
    'idas/IDASet.m'...
    'idas/IDASetB.m'...
    'idas/IDAFree.m'...
         };

ida_ftp = {
    'idas/function_types/IDAResFn.m'...
    'idas/function_types/IDASensResFn.m'...
    'idas/function_types/IDAQuadRhsFn.m'...
    'idas/function_types/IDARootFn.m'...
    'idas/function_types/IDADenseJacFn.m'...
    'idas/function_types/IDABandJacFn.m'...
    'idas/function_types/IDAJacTimesVecFn.m'...
    'idas/function_types/IDAPrecSetupFn.m'...
    'idas/function_types/IDAPrecSolveFn.m'...
    'idas/function_types/IDAGcommFn.m'...
    'idas/function_types/IDAGlocalFn.m'...
    'idas/function_types/IDAMonitorFn.m'...
    'idas/function_types/IDAResFnB.m'...
    'idas/function_types/IDAQuadRhsFnB.m'...
    'idas/function_types/IDADenseJacFnB.m'...
    'idas/function_types/IDABandJacFnB.m'...
    'idas/function_types/IDAJacTimesVecFnB.m'...
    'idas/function_types/IDAPrecSetupFnB.m'...
    'idas/function_types/IDAPrecSolveFnB.m'...
    'idas/function_types/IDAGcommFnB.m'...
    'idas/function_types/IDAGlocalFnB.m'...
    'idas/function_types/IDAMonitorFnB.m'...
         };

kin_fct = {
    'kinsol/KINSetOptions.m'...
    'kinsol/KINInit.m'...
    'kinsol/KINSol.m'...
    'kinsol/KINGetStats.m'...
    'kinsol/KINFree.m'...
          };

kin_ftp = {
    'kinsol/function_types/KINSysFn.m'...
    'kinsol/function_types/KINDenseJacFn.m'...
    'kinsol/function_types/KINBandJacFn.m'...
    'kinsol/function_types/KINJacTimesVecFn.m'...
    'kinsol/function_types/KINPrecSetupFn.m'...
    'kinsol/function_types/KINPrecSolveFn.m'...
    'kinsol/function_types/KINGcommFn.m'...
    'kinsol/function_types/KINGlocalFn.m'...
    };

nvec_fct = {
    'nvector/N_VDotProd.m'...
    'nvector/N_VL1Norm.m'...
    'nvector/N_VMax.m'...
    'nvector/N_VMaxNorm.m'...
    'nvector/N_VMin.m'...
    'nvector/N_VWL2Norm.m'...
    'nvector/N_VWrmsNorm.m'...
           };
putils_fct = {
    'putils/mpirun.m'...
    'putils/mpiruns.m'...
    'putils/mpistart.m'...
             };
cv_es = {
    'cvodes/examples_ser/mcvsAdvDiff_bnd.m'...
    'cvodes/examples_ser/mcvsDiscRHS_dns.m'...
    'cvodes/examples_ser/mcvsDiscSOL_dns.m'...
    'cvodes/examples_ser/mcvsDiurnal_kry.m'...
    'cvodes/examples_ser/mcvsHessian_FSA_ASA.m'...
    'cvodes/examples_ser/mcvsOzone_FSA_dns.m'...
    'cvodes/examples_ser/mcvsPleiades_non.m'...
    'cvodes/examples_ser/mcvsPollut_FSA_dns.m'...
    'cvodes/examples_ser/mcvsRoberts_ASAi_dns.m'...
    'cvodes/examples_ser/mcvsRoberts_dns.m'...
    'cvodes/examples_ser/mcvsRoberts_FSA_dns.m'...
    'cvodes/examples_ser/mcvsVanDPol_dns.m'...
        };

cv_ep = {
    'cvodes/examples_par/mcvsAdvDiff_FSA_non_p.m'...
    'cvodes/examples_par/mcvsAtmDisp_kry_bbd_p.m'...
    'cvodes/examples_par/mcvsDecoupl_non_p.m'...
        };

ida_es = {
    'idas/examples_ser/midasBruss_ASA_dns.m'...
    'idas/examples_ser/midasBruss_dns.m'...
    'idas/examples_ser/midasHeat2D_bnd.m'...
    'idas/examples_ser/midasPendI1_dns.m'...
    'idas/examples_ser/midasPendI2_dns.m'...
    'idas/examples_ser/midasReInit_dns.m'...
    'idas/examples_ser/midasRoberts_ASAi_dns.m'...
    'idas/examples_ser/midasRoberts_dns.m'...
    'idas/examples_ser/midasSlCrank_dns.m'...
    'idas/examples_ser/midasSlCrank_FSA_dns.m'...
         };

kin_es = {
    'kinsol/examples_ser/mkinDiagon_kry.m'...
    'kinsol/examples_ser/mkinFerTron_dns.m'...
    'kinsol/examples_ser/mkinRoboKin_dns.m'...
    'kinsol/examples_ser/mkinTest_dns.m'...
         };

kin_ep = {
    'kinsol/examples_par/mkinDiagon_kry_p.m'...
         };