# -----------------------------------------------------------------
# $Revision: 1.6 $
# $Date: 2012-10-29 21:39:55 $
# -----------------------------------------------------------------
# Programmer(s): Radu Serban @ LLNL
# -----------------------------------------------------------------
# Copyright (c) 2002, The Regents of the University of California.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see sundials/kinsol/LICENSE.
# -----------------------------------------------------------------
# Makefile for KINSOL documents
# -----------------------------------------------------------------

QUIET =

SHARED_DOC = ../sundials

HTML_DIR = ../../html/documentation

UG_FILES = \
kin_guide.tex \
kin_intro.tex \
kin_install.tex \
kin_math.tex \
kin_use.tex \
kin_fcmix.tex \
kin_nvec.tex \
kin_org.tex \
kin_newlinsolv.tex \
kin_linsolv.tex \
kin_constants.tex

UG_SHARED_FILES = \
install.tex \
linsolv.tex \
nvec_generic.tex \
nvec_parallel.tex \
nvec_serial.tex \
nvec_openmp.tex \
nvec_pthreads.tex \
sun_org.tex \
types.tex \
ug.tex \
biblio.bib \
texi2dvi 
#ug_html.tex \
#processHTML

UG_SHARED_FIGURES = \
bandmat.eps \
cscmat.eps \
sunorg1.eps \
sunorg2.eps \
doc_logo.eps

UG_CMAKE_FIGURES = \
ccmakedefault.eps \
ccmakeempty.eps \
ccmakeprefix.eps \
cmaketest.eps

EX_FILES = \
kin_examples.tex \
kin_ex_intro.tex \
kin_ex_c.tex \
kin_ex_fortran.tex \
kin_ex_codes.tex

EX_SHARED_FILES = \
ug.tex \
biblio.bib \
texi2dvi \
#ug_html.tex \
#processHTML

#HTML_BUTTONS =\
#back.png \
#contents.png \
#index.png \
#next.png \
#next_g.png \
#prev.png \
#prev_g.png \
#up.png \
#up_g.png \
#warning.png

#-------------------------------------------------------------------------------

all:
	@echo "Usage:"
	@echo "  make ug     - compile LaTeX files to generate DVI file for KINSOL User's Guide"
	@echo "  make ug_pdf - compile LaTeX files to generate PS + PDF files for KINSOL User's Guide"
	@echo "  make ex     - compile LaTeX files to generate DVI file for KINSOL Examples Guide"
	@echo "  make ex_pdf - compile LaTeX files to generate PS + PDF files for KINSOL Examples Guide"
	@echo "  make clean  - remove all temporary files"
	@echo "  make purge  - remove temporary files + PS + PDF files"
	@echo ""
install: ug_pdf ex_pdf
#install: ug_pdf ex_pdf ug_html ex_html
#	rm -rf $(HTML_DIR)/kin_guide
#	rm -rf $(HTML_DIR)/kin_examples
#	cp kin_guide.ps $(HTML_DIR)
	cp kin_guide.pdf $(HTML_DIR)
#	cp -r kin_guide $(HTML_DIR)
#	cp kin_examples.ps $(HTML_DIR)
	cp kin_examples.pdf $(HTML_DIR)
#	cp -r kin_examples $(HTML_DIR)

#-------------------------------------------------------------------------------

ug: 	copy_ug copy_ug_figs copy_ug_cmake_figs kin_guide.aux

ug_pdf: ug kin_guide.ps

#ug_html: ug
#	cp ug_html.tex ug.tex
#	latex2html -split 3 -link 3 -info 1 -no_math -html_version 3.2,math -short_index -show_section_numbers -local_icons -white -notransparent kin_guide.tex
#	@for i in ${HTML_BUTTONS} ; do \
#	cp ${SHARED_DOC}/buttons/$${i} kin_guide/ ;\
#	done
#	@cp ${SHARED_DOC}/l2h_styles.css kin_guide/kin_guide.css
#	./processHTML kin_guide

copy_ug:
	@for i in ${UG_SHARED_FILES} ; do \
	cp ${SHARED_DOC}/$${i} . ;\
	done

copy_ug_figs:
	@for i in ${UG_SHARED_FIGURES} ; do \
	cp ${SHARED_DOC}/figures/$${i} . ;\
	done

copy_ug_cmake_figs:
	@for i in ${UG_CMAKE_FIGURES} ; do \
	cp ${SHARED_DOC}/figures/cmake/$${i} . ;\
	done

kin_guide.aux: ${UG_FILES} ${UG_SHARED_FILES:%=${SHARED_DOC}/%}
	texi2dvi ${QUIET} kin_guide.tex

kin_guide.ps: kin_guide.dvi
	dvips ${QUIET} -Ppdf kin_guide
	ps2pdf kin_guide.ps

#-------------------------------------------------------------------------------

ex:	ug copy_ex kin_examples.aux

ex_pdf: ex kin_examples.ps

#ex_html: ex
#	cp ug_html.tex ug.tex
#	latex2html -split 4 -link 4 -info 1 -no_math -html_version 3.2,math -show_section_numbers -local_icons -white -notransparent kin_examples.tex
#	@for i in ${HTML_BUTTONS} ; do \
#	cp ${SHARED_DOC}/buttons/$${i} kin_examples/ ;\
#	done
#	@cp ${SHARED_DOC}/l2h_styles.css kin_examples/kin_examples.css
#	./processHTML kin_examples

copy_ex:
	@for i in ${EX_SHARED_FILES} ; do \
	cp ${SHARED_DOC}/$${i} . ;\
	done

kin_examples.aux: ${EX_FILES} ${EX_SHARED_FILES:%=${SHARED_DOC}/%}
	texi2dvi ${QUIET} kin_examples.tex

kin_examples.ps: kin_examples.dvi
	dvips ${QUIET} -Ppdf kin_examples
	ps2pdf kin_examples.ps

#-------------------------------------------------------------------------------

clean:
	rm -rf ${UG_SHARED_FILES}
	rm -rf ${UG_SHARED_FIGURES}
	rm -rf ${UG_CMAKE_FIGURES}
	rm -rf ${EX_SHARED_FILES}
	rm -f *~ *.aux *.bbl *.blg *.log *.toc *.lot *.lof *.ind *.ilg *.idx *.dvi *.out

purge:  clean
	rm -f kin_guide.ps kin_guide.pdf
	rm -f kin_examples.ps kin_examples.pdf
	rm -rf kin_guide kin_examples
