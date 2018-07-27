
TEXFILES = examples/A01.tex examples/A02.tex examples/A03.tex examples/B01.tex examples/C01.tex
FIGS = figs/structure.pdf figs/transaction.pdf


user-manual.pdf: user-manual.tex $(TEXFILES) $(FIGS) decls-common.tex decls-handout.tex
	pdflatex user-manual
	pdflatex user-manual

clean:
	rm -f *~ *.pdf *.log
	rm -rf response_files source_files
