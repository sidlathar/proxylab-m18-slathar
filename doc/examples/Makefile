FORMAT = ./txt2tex

TEXFILES = A01.tex A02.tex A03.tex B01.tex C01.tex

all: $(TEXFILES)

A01.tex: A01-request-text.cmd
	$(FORMAT) -nc -f A01-request-text.cmd -o A01

A02.tex: A02-fetch-data.cmd
	$(FORMAT) -nc -f A02-fetch-data.cmd -o A02

A03.tex: A03-fetch-missing.cmd
	$(FORMAT) -nc -f A03-fetch-missing.cmd -o A03

B01.tex: B01-out-of-order.cmd
	$(FORMAT) -nc -f B01-out-of-order.cmd -o B01

C01.tex: C01-caching.cmd
	$(FORMAT) -nc -f C01-caching.cmd -o C01

clean:
	rm -f *~ *.tex
	rm -rf response_files source_files

