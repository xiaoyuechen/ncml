PDF_FILE := report.pdf

all: $(PDF_FILE)

%.pdf: %.tex bibliography.bib
	pdflatex $(subst .tex,,$<)
	bibtex $(subst .tex,,$<)
	pdflatex $(subst .tex,,$<)
	pdflatex $(subst .tex,,$<)

zip: UU_logo.pdf bibliography.bib placeholder.jpg report.tex
	@zip report_template.zip $^

.PHONY:clean
clean:
	rm -f *aux
	rm -f *out
	rm -f *log
	rm -f *bbl
	rm -f *blg
	rm -f *.bcf
	rm -f *.run.xml
	rm -f report_template.zip
	rm -f $(PDF_FILE)
