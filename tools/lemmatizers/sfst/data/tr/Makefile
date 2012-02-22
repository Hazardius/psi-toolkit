LEXFILES = lexicon/adjectives lexicon/adverbs lexicon/conjunctions\
			lexicon/interjections lexicon/nouns lexicon/postpositions\
			lexicon/proper_nouns lexicon/verbs lexicon/misc
FSTFILES = trmorph.fst ninfl.fst vinfl.fst num.fst symbols.fst
SOURCES = $(LEXFILES) $(FSTFILES)
SUBDIRS=phon

include Makefile.inc

.PHONY: all subdirs $(SUBDIRS)

all: trmorph.a

trmorph.a: subdirs trmorph.fst symbols.fst vinfl.fst ninfl.fst deriv.a
deriv.a:  subdirs num.a symbols.fst $(LEXFILES)

subdirs: 
	for dir in $(SUBDIRS); do  $(MAKE) -C $$dir;  done

archive:
	./archive.sh $(DISTNAME)

testset: ../data/data
	awk '{print $$2}' ../data/data |sort |uniq > tests.all

clean:
	-rm -f *.a *~ Makefile.bak tests.all 
	-for dir in $(SUBDIRS); do  $(MAKE) -C $$dir clean; done

#Makefile: *.fst
#	-makedepend -Y -o.a $(SOURCES) 2>/dev/null 

test:
	fst-mor trmorph.a  < testset.1 |tee /tmp/trfst-testset1.out|less; \
		(echo -n `date`" "; grep 'no result' </tmp/trfst-testset1.out|wc -l) >> .testset1-results

# DO NOT DELETE

trmorph.a: symbols.fst vinfl.fst ninfl.fst
ninfl.a: symbols.fst
num.a: symbols.fst
deriv.a: symbols.fst ninfl.fst
