all:
	

docs.pdf:
	cd docs && pdflatex -shell-escape docs.tex
	cd docs && pdflatex -shell-escape docs.tex
	mv docs/docs.pdf .

docker:
	docker build -t epigram .
