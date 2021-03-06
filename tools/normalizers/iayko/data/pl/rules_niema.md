# Dictionary

## Verb categories.
## VerbFin is a category, which is annotated as "fin" in Polimorf, i.e. future or present tense of verbs.
    VerbFin = LoadFstFromFar['polimorf/polimorf.fin.far','MAIN'] ;

## VerbPraet is a category, which is annotated as "praet" in Polimorf
    VerbPraet = LoadFstFromFar['polimorf/polimorf.praet.far','MAIN'] ;

## VerbFin is a category, which is annotated as "fin" in Polimorf, i.e. future or present tense of verbs.
    VerbInf = LoadFstFromFar['polimorf/polimorf.inf.far','MAIN'] ;

## Verb is a category for all types of Verbs

    Verb = VerbFin | VerbPraet | VerbInf ;

## Pred is a category, which is annotated as "pred" in Polimorf (modal verbs, e.g. 'wolno')
    Pred = LoadFstFromFar['polimorf/polimorf.pred.far','MAIN'] ;

## Pact is a category, which is annotated as "pact" in Polimorf (actice participles, e.g. 'mający')
    Pact = LoadFstFromFar['polimorf/polimorf.pact.far','MAIN'] ;

##  Ppas category is the category for past participles such as "zadowolony"

    Ppas = LoadFstFromFar['polimorf/polimorf.ppas.far','MAIN'] ;

##  Pant is the category for participles such as "zaniósłszy"

    Pant = LoadFstFromFar['polimorf/polimorf.pant.far','MAIN'] ;

##  Ger is the category for gerund such as "osiągnięcie"

    Ger = LoadFstFromFar['polimorf/polimorf.ger.far','MAIN'] ;

## Adjective categories
## Inst is the category for instrumental forms

    Adj = LoadFstFromFar['polimorf/polimorf.adj.far','MAIN'] ;

    Inst = LoadFstFromFar['polimorf/polimorf.inst.far','MAIN'] ;

## Roman numerals - introduced in order to distinguish Roman numerals from strings
    RomanNumeral = ("I"|"V"|"X"|"L"|"C"|"D"|"M") ;

## Two technical "PreRules"

    PrefixY = Optimize[
                         ( Aa"rc" | Aa"nt" | Dd"ec" | Ee"ur" | Ff"luor" | Kk"alor" |
                         Mm"iędz" | Mm"ęcz" | Nn"ib" |Oo"ks" | Oo"pt" | Pp"er" |
                         Pp"rz" |Ss"cynt" | Ss"ekst" |Tt"ach" | Tt | Ww | Nn"iew" |
                         Pp"oprz" | Pp"ow"
                         )
                        ] ;

The rule should not change 'y; to 'j' if it ends a prefix. Technically, each 'y' that should remain untouched is sorrounded by "dollars".

    export PreRuleFixY = 
                    PrefixY
                        ("y" : "$y$") 
                             Vowel
                               NonEmptyString ;

## Rule: fix_i: 
The rule should not change 'i' to 'j' if it follows a word. Technically, each 'yi that should remain untouched is sorrounded by "dollars".

    PrefixI = Optimize[
                         ( Nn"ie" | Gg"dzie" )
                      ] ;

    export PreRuleFixI = 
                    PrefixI
                        ("i" : "$i$") 
                             NonEmptyString ;

############# BEGINNING OF THE RULES #############################
#### Rules motivated by Feliński's article (1816)

## Rule: Remove 'z' from 'źrz"
The rule removes 'z' form "źrzódło" etc. The pair "źrzódło->źródło" is temporarily removed from tests as the word "źrzódło" is present in the contemporary dictionary.
> śrzoda -> środa

    Remove_z_from_zrz = (
                            ( "śrz" : "śr" ) |
                            ( "Śrz" : "Śr" ) |
                            ( "źrz" : "źr" ) |
                            ( "Źrz" : "Źr" )
                        ) ;

    export Rule1800 = Optimize[
           CDRewrite[ Remove_z_from_zrz,
                      "",
                      "",
                      Any* ] ];

## Rule: Letter 'i after "y" before consonant
The rule is controversial, as it generates too many exceptions.
> przyimować -> przyjmować
> wyimować -> wyjmować

    Replace_i_with_j =
                         ("i" : "j") ;

    export Rule1823 = Optimize[
           CDRewrite[ Replace_i_with_j,
                      ("y" | "$y$"),
                      "mow",
                      Any* ] ];

## Rule: Letter 'i' at the beginning before vowel
The rule replaces 'i', occurring at the beginning of the word and before vowel, with 'j'.
> iayko -> jajko

    Jota_at_beginning =
                         ( ("i" : "j") | ("I" : "J") ) ;                            

    export Rule1826 = Optimize[
           CDRewrite[ Jota_at_beginning,
                      Beginning,
                      (Vowel - Ii) NonEmptyString,
                      Any* ] ];

## Rule: Letter 'i' between vowels
The rule replaces 'i', occurring between vowels, with 'j'.
> maia -> maja 

    export Rule1828 = Optimize[
           CDRewrite[ Replace_i_with_j,
                      Vowel,
                      Vowel,
                      Any* ] ];

## Rule: Letter 'i' after vowel other than "y"
The rule replaces 'i' occurring after vowel other than 'y' before consonant. Exception: "i" should not follow either "gdzie" or "nie"

> boi -> boi
> boika -> bojka
> gdzieindziej -> gdzie indziej
> nieintuicyjny -> nieintuicyjny
                                                           
    export Rule1894 = Optimize[
           CDRewrite[ Replace_i_with_j,
                      Vowel - "y",
                      Consonant,
                      Any* ] ];

#### Rules that change y into j

## Rule: Letter 'y' before vowel other than 'i'
The rule replaces 'y', occurring before vowel other than 'i', with 'j'.The rule is not confirmed in the corpus.
Attention: The rule should not be used for prefixes ending with 'y', e.g. "arcy"
> Yabłko -> Jabłko
> swoye -> swoje
> arcyatrakcja -> arcyatrakcja

    Replace_yY_with_jJ = ( ("y"  : "j") |  ("Y" : "J") ) ;
                               
    export Rule1816_01 = Optimize[
           CDRewrite[ Replace_yY_with_jJ,
                      "",
                      Vowel - "i",
                      Any* ] ];

## Rule: Letter 'y' before 'i'
In a context VyiC 'y' disappears.
The rule also processes the word "szyyi", which is removed from tests, as it is finally and uncorrectly converted into "szji" according to rule 005_01. 
The rule is not confirmed by the corpus, as the word "krayina" is very rare. 
The rule should not affect words ending with "yi", e.g. "kolacyi", which are processed by another rule. 

> krayina -> kraina
> stoyisz -> stoisz
> kolacyi -> kolacji

    Replace_yi_with_i =  Vowel
                         ("yi": "$i$")
                              Consonant;

    export Rule1816_02 = Optimize[
           CDRewrite[ Replace_yi_with_i,
                      NonEmptyString,
                      "",
                      Any* ] ];

## Rule: Letter 'y' after vowel
The rule replaces 'y' occurring after vowel with 'j'; y must not be followed by "i"
> bayka -> bajka
> kray -> kraj

    Replace_y_with_j =
                             ("y" : "j") ;
    
    export Rule1830 = Optimize[
           CDRewrite[ Replace_y_with_j,
                      Vowel,
                      ( Letter - "i" | End ),
                      Any* ] ];

## Rule: Infinitive 'ź_into_ć'
This rule replaces 'ź' that ends verb infinitives with 'ć'.
The words "wiedź" and "kładź" have been removed from tests, as they exist in modern vocabulary

> bydź -> być
> uledz -> ulec
> piedz -> piec
> módz -> móc
> przysiądz -> przysiąc

    InfinitiveEnding =
                "y" ("dź" : "ć")           |
                ("ie" | "a") ("dź" : "ść") |
                ("ą" | "e"  | "ó") ("dz" : "c") ;

    export Rule1832_A = Optimize[
           CDRewrite[ InfinitiveEnding,
                      "",
                      End,
                      Any* ] ];

## Rule: PrefixPreposition
The below rule handles prepositions that precede nouns.
> beskresie -> bezkresie
> roskoszy -> rozkoszy

    PrefixPreposition =
                             Bb("eskres" : "ezkres") |
                             Rr("oskosz" : "ozkosz") ;

    export Rule1840 = Optimize[
           CDRewrite[ PrefixPreposition,
                      Beginning,
                      "",
                      Any* ] ];
                                               
## Rule: A_acute  (the letter no longer exists in Polish save for foreign words)
This rule replaces 'a_acute' with standard 'a'. Existing words are listed in the exceptions list.

> á->a

    A_acute = (
                  ("Á" : "A") |
                  ("á" : "a")
              ) ;

    export Rule1816_03 = Optimize[
           CDRewrite[ A_acute,
                      "",
                      "",
                      Any* ] ];


## Rule: Imperative 'iej'
This rule replaces imperatives ending "iej" into "ij"; "rzej"" into "rzyj". The corpus does not confirm the need for the rule (very rare occurrencies of ancient spelling).
The word "zwiej" has been removed from tests, as it exists in modern vocabulary

> rwiej -> rwij
> tniej -> tnij
> drzej -> drzyj

    ImperativeEnding =
                      ("tniej" : "tni$j$")       |
                      ("rwiej" : "rwi$j$")       |
                      ("drzej" : "drzy$j$")  ;

    export Rule1816_04 = 
                           ImperativeEnding ;

## Rule: BarePreposition
These rules replace 's' at the end of a preposition with 'z'. The corpus does not confirm the need for the rule (very rare occurrencies of ancient spelling).
The below rule handles bare prepositions.
> bes -> bez
> przes -> przez

    BarePreposition =
                     Bb ("es" : "ez")        |
                     Pp ("rzes" : "rzez" )   ;

    export Rule1816_05 = Optimize[
           CDRewrite[ BarePreposition,
                      Beginning,
                      End,
                      Any* ] ];

#### Delegacyja Rules (1830)
These are rules introduced by Akademia Umiejętności in 1830

## Rule: Past Participle "łszy"
The rule inserts 'ł' before "szy" in past participle forms.
Rule002_05 checks if the resulting string is tagged as 'pant' in Polimorf.
The rule is not supported in the corpus.
> poszedszy -> poszedłszy
> usiadszy -> usiadłszy
> zjadszy -> zjadłszy
> zdarszy -> zdarłszy
> zaniósszy -> zaniósłszy
> znalazszy -> znalazłszy

    PantEnding =    (
                         ( "dszy" : "dłszy" ) |
                         ( "rszy" : "rłszy" ) |
                         ( "sszy" : "słszy" ) |
                         ( "zszy" : "złszy")
                     ) ;

    export Rule1792 =
                      (NonEmptyString
                          PantEnding)
    %pant

## Rule: Polonization of foreign names

The rule treats foreign names listed in the dictionary transducer reverts the original spelling of foreign names.
The rule changes Marija into Marya, which then finally is converted into Maria. Other chanes are not confirmed in the corpus.
"Maryja->Maria" is excluded form the tests as "Maryja" is present in the contemporary dictionary. Te rule is not confirmed in the corpus.
> Marija -> Maria
> Julija -> Julia
> Fabijan -> Fabian
> Scypijon -> Scypion
> Grecyja -> Grecja

    ForeignNames = (
                       ( "Marij" : "Mari" )       |
                       ( "Maryj" : "Mari" )       |
                       ( "Julij" : "Juli" )       |
                       ( "Fabijan" : "Fabian" )   |
                       ( "Scypijon" : "Scypion" ) |
                       ( "Nyobe" : "Niobe" )      |
                       ( "Grecyj" : "Grecy")
                   ) ;

    export Rule1830_01 = Optimize[
           CDRewrite[ ForeignNames,
                      Beginning,
                      AnyString,
                      Any* ] ];

## Rule: Foreign words with "gie"
The rule deletes 'i' from  "gie" in the foreign words. The rule is not confirmed in the corpus.
> gienerał -> generał
> gieniusz -> geniusz
> Eugieniusz -> Eugeniusz
> Gienewy -> Genewy
> ewangielicki > ewangelicki

    Replace_gie_with_ge = (
                                 ( "Eugieniusz" : "Eugeniusz" )    |
                                 Ee ("wangieli" : "wangeli" )      |
                                 Gg ("ie" : "e") "nerał"           |
                                 Gg ("ie" : "e") "niusz"           |
                                 Gg ("ie" : "e") "o"               |
                                 Gg ("ie" : "e") "nealog"          |
                                 ("Gie" : "Ge") "rman"             |
                                 ("Gie" : "Ge") "new"              |
                                 ("Gie" : "Ge") "rtrud"
                            ) ;

    export Rule1830_02 = Optimize[
           CDRewrite[ Replace_gie_with_ge,
                      Beginning,
                      AnyString,
                      Any* ] ];

# The rule for 'ó'
The rule changes 'ó' to 'o' in words listed in the dictionary transducer O_acute. The rule is not supported by the corpus.

> aktór -> aktor
> honór > honor
> Wiktór -> Wiktor
> fawór -> fawor
> dzięciół -> dzięcioł
> gruczół -> gruczoł
> mozóły -> mozoły

    O_acute = Optimize [
               (
                  Aa ( "któr" : "ktor" )         |
                  Hh ( "onór" : "onor" )         |
                  Ff ( "awór" : "awor" )         |
                  Jj (  "akób" : "akob" )        |
                  Ww  ( "iktór" : "iktor" )      |
                  Dd ( "zięció" : "zięcio" ) ( "ł" | "l" ) |
                  Gg ( "ruczó": "ruczo" )    ( "ł" | "l" ) |
                  Mm ( "ozó" : "ozo" )       ( "ł" | "l" ) Vowel
               ) ] ;

    export Rule1830_03 =
                        O_acute
                                AnyString;

## Rule: "x-> ks"
The rule replaces obsolete x with ks in sepecified contexts: 

> xiążę -> książę
> expedycja -> ekspedycja
> excesarz -> ekscesarz
> XI -> XI
> X -> X

    Replace_x_with_ks = (
                            ("X":"Ks") | ("x" : "ks")
                       ) ;

    export Rule1838 =  Optimize[
            CDRewrite[ Replace_x_with_ks,
                      "",
                      Lower - "e",
                      Any* ] ];

## Rule: "adjective_endings"
The rule replaces obsolete adjective endings: "iemi" with "imi"; "emi" with "ymi"; "em" with "ym"
The rule is turned off as the compilation takes too long. Surprisingly, the rule is not confirmed in the corpus.
> wysokiemi -> wysokimi
> długiemi -> długimi
> nowemi -> nowymi
> dobremi -> dobrymi
> całkowitem -> całkowitym
# > z niemi -> z nimi - wyłączone z racji obecności w słowniku wyjątków

    AdjectiveEnding = (
                            (("i"|"j")"emi" : "imi")    |
                            (Consonant - "j")("emi" : "ymi")   |
                            (("i"|"j")"em" : "im") |
                            (Consonant - "j")("em" : "ym")
                        ) ;
    export Rule1936_A =
                         ( NonEmptyString
                             AdjectiveEnding |
                             ("tem" : "tym") | ("drugiem" : "drugim") | ("niemi" : "nimi") ) ;
    
    %adj&case=inst

# Akademia Rules (1891)
These are rules introduced by Akademia Umiejętności in 1891

## Rule: "z" replaced by "s" in adjective endings: "izki"

> męzkiego > męskiego
> papiezką > papieską

    Ending_zki = 
                  ( ("ą"|"i"|"ę"|"e")
                             ("zk" : "sk") ) ;
                   
    export Rule1889 = Optimize[
           CDRewrite[ Ending_zki,
                      "",
                      ("a" | "i" | "ą" | "e"),
                      Any* ] ];

## Rule: "z" replaced by "s" in nouns ended by "ęstwo"
> zwycięztwo -> zwycięstwo

    Ending_eztwo = 
                  ("ęztw" : "ęstw") ;
                   
    export Rule1891 = Optimize[
           CDRewrite[ Ending_eztwo,
                      "",
                      "",
                      Any* ] ];

## Rule: "z" replaced by "s" in some particles
> zkąd -> skąd
> ztamtąd -> stamtąd
> zchodzić -> schodzić
> ztężeć -> stężeć
> ztok -> stok
> zpływać -> spływać

    Preposistion_z =     ( 
                   ("Z": "S") | ("z" : "s")
                   )
                   ("ch" | "f" | "k" | "p" | "t" ) ;
                   
    export Rule1896_A = Optimize[
           CDRewrite[ Preposistion_z,
                      Beginning,
                      AnyString,
                      Any* ] ];


## Rule: E_acute  (the letter no longer exists in Polish)
This rule replaces 'e_acute' with standard 'e'.

    E_acute = (
                  ("É" : "E") |
                  ("é" : "e")
              ) ;

    export Rule1900 = Optimize[
           CDRewrite[ E_acute,
                      "",
                      "",
                      Any* ] ];

# Rules by Akademia Umiejętności in Cracow, 1918
## Rule: palatalization of preposistion 'z' in verbs
The rule converts 'z' into 'ś' in verbs starting with 'ci'. The corpus does not confirm the need for the rule (very rare occurrencies of ancient spelling).
> zcierpieć -> ścierpieć
> zcisnąć -> ścisnąć

    Beginning_zci =    ( 
                         ("z" : "ś") | 
                         ("Z" : "Ś") 
                       )
                           "ci" ;

    export Rule1918_01 = Optimize[
           CDRewrite[ Beginning_zci,
                      Beginning,
                      NonEmptyString,
                      Any* ] ];

# Rules from 1936

## Rule: Ending "yjum" changed into "ium"
The rule replaces endings: "yjum" or "ium" with "ium".

> misteryjum -> misterium
    Ending_ium = 
                  ("yjum" : "ium") ;

    export Rule1867 = Optimize[
           CDRewrite[ Ending_ium,
                      Followed_by_i,
                      End,
                      Any* ] ];

## Rule: Ending "yjum" changed into "jum"
The rule replaces endings: "yjum" or "ium" with "jum".

> hospicyjum -> hospicjum
    Ending_jum = 
                  (("yjum" | "ium") : "jum") ;


    export Rule1893 = Optimize[
           CDRewrite[ Ending_jum,
                      Followed_by_j,
                      End,
                      Any* ] ];

## Rule: Ending_ija
The rule replaces ending "yja" with "ia". 
The words 'historyj', 'religij', 'geografij' have been removed from tests, as they exist
> historyja -> historia
> historyje -> historie
> religija -> religia


    Ending_ija =
                  ( ( "yj" | "ij" ) : "i" )
                           ("a" | "i" | "ę" | "ą" | "e" | "om" | "ami" | "ach" | "ów") ;

    export Rule1892 = Optimize[
           CDRewrite[ Ending_ija,
                      Followed_by_i,
                      End,
                      Any* ] ];

## Rule: Ending_ij
The rule replaces ending "yj" with "ii"
> kopyj -> kopii
> religij -> religii

    Ending_ij =
                  ( ( "yj" | "ij" ) : "ii" ) ;

    export Rule1910 = Optimize[
           CDRewrite[ Ending_ij,
                      Followed_by_i,
                      End,
                      Any* ] ];

## Rule: Ending_yja
The rule replaces endings: "yja" with "ja".
The words 'decyzyj', 'lekcyj', 'dygresyj' have been removed from tests, as they exist in modern vocabulary
> decyzyja -> decyzja
> decyzyje -> decyzje
> lekcyja -> lekcja
> dygressyja -> dygresja

    Ending_yja =
                   (("yj" | "y") : "j") 
                      ("a" | "i" | "ę" | "ą" | "e" | "om" | "ami" | "ach" | "ów") ;

    export Rule1903 = Optimize[
           CDRewrite[ Ending_yja,
                      Followed_by_j,
                      End,
                      Any* ] ];

## Rule: Ending_yj
The rule changes "yj" with "ji".
> dygressyj -> dygresji

    Ending_yj = 
                 ( ( "yj" | "yi") : "ji") ;

    export Rule1915 = Optimize[
           CDRewrite[ Ending_yj,
                     Followed_by_j,
                      End,
                      Any* ] ];


## After 1936 it was agreed that after all consonants (except for "c', "s", "z"), "y" or "j" are replaced by "i"
> Danya -> Dania
> Danja -> Dania
> antyandegaweński -> antyandegaweński

    Replace_j_with_i = 
                  ( ("y" | "j") : "i" ) ;

    export Rule1936_B = Optimize[
           CDRewrite[ Replace_j_with_i,
                      Followed_by_i,
                      Vowel,
                      Any* ] ];


########## Division rules #################
Below are rules that divide words. 

## The below rule handles verbs preceded by particle "nie"
> niechciał -> nie chciał
> niemódz -> nie móc

    NieVerb = Nn"ie"
                    ("" : " ")
                              NonEmptyString ;
    %*+verb

    export Rule1796 = 
                        NieVerb ;

## Specific rules for word division 
These are rules that divide words into parts (usually: preposition + noun)
 
> bezmała -> bez mała
> nadewszystko -> nade wszystko
> zdala -> z dala
> nakoniec -> na koniec
> gdzieindziej -> gdzie indziej

## List of words that should be changed into the separate spelling

    SeparateWord =
                    ( "bez"    ("": " ") ("mała" | "wątpienia") |
                      "bądź"   ("": " ") ("to" | "też") |
                      "beze"   ("": " ") "mnie" |
                      "do"     ("": " ") ("cna" | "syta" | "niedawna") |
                      "na"     ("": " ") ("pewno" | "czczo" | "jaw" | "ogół" | "podorędziu" | "przemian" | "zabój" | "koniec" | "ostatek" ) |
                      "nade"   ("": " ") "wszystko" |
                      "nie"    ("": " ") ("tylko" | "zawsze" ) |
                      "od"     ("": " ") ("dawna" | "niechcenia" | "razu" ) |
                      "ode"    ("": " ") "mnie" |
                      "po"     ("": " ") ("cichu" | "ciemku" | "kryjomu" | "społu" | "trochu" | "troszku") |
                      "pode"   ("": " ") "mną" |
                      "przede" ("": " ") ("dniem" | "wszystkim") |
                      "przez"  ("": " ") "co" |
                      ("tem" : "tym") ("": " ") "bardziej" |
                      "w"      ("": " ") "ogóle" |
                      "we"     ("": " ") "mnie" |
                      "z"      ("": " ") ("powodu" | "pomocą" | "cicha" | "głupia" | "daleka" | "dala" ) |
                      "za"     ("": " ") ("mąż" | "pomocą" | "świeża" | "widna" ) |
                      "ze"     ("": " ") "mną" |
                      "gdzie"  ("$i$": " i") "ndziej"
                    ) ;
    
    export Rule1929 = 
                      SeparateWord ;

## The problem appeared with prepositions: "potem", "zaczem":
## The below rule handles prepositions ending with "czem".
The words 'zaczem' and 'poczem' have been removed from tests, as they exist in modern vocabulary
> przyczem -> przy czym
> przytem -> przy tym

    SeparatePreposition = Optimize [
                             "za"   ("":" ") ("czem":"czym")   |
                             "na"   ("":" ") ("tem":"tym")     |
                             ("zatym" : "zatem")               |
                             "o" ("":" ") ( ("czem"| "czym") :"czym") | 
                             "o" ("":" ") ( ("tem" | "tym") :"tym")   |
                             "po"   ("":" ") ( ("czem" | "czym" ) : "czym")  |
                             "przy" ("":" ") ( ("czem"| "czym") :"czym") | 
                             "przy" ("":" ") ( ("tem" | "tym") :"tym")   |
                             "przy" ("":" ") ( ("niem" | "nim") : "nim") |
                             "w" ("":" ") ( ("czem"| "czym") :"czym") | 
                             "w" ("":" ") ( ("tem" | "tym") :"tym")   |
                             "w" ("":" ") ( "tej" :"tym") 
                             ] ;

    export Rule1939_A = 
                             SeparatePreposition ;

## Particle "by" 
"By" particles and their inflected forms should be separated from particles and conjunctions

> alboby-> albo by
> czyby -> czy by
> któremuby -> któremu by
> możnaby > można by
> powinnoby -> powinno by

## Adjective Pronouns that should be separated from 'by'
    AdjectivePronoun = Optimize [
                      ("jaki" | "który" |
                       "jakiego" | "którego" |
                       "jakiemu" | "któremu" |
                       "jakiej" | "której" |
                       "jakim"  | "którym" |
                       "jaką"  | "którą" |
                       "jakie" | "które" |
                       "jacy" | "którzy" |
                       "jakich" | "których" |
                       "jakimi" | "którymi" |
                       "jakim" | "którym" )
                       ] ;

## Particles that should be separated from 'by'
    Particle_by = Optimize [
                             ( AdjectivePronoun | Aa"lbo" | Aa"lbowiem" | Aa"le" | Aa"leż" | Aa"ż" | Bb"o" |
                               Bb"owiem" | Bb"yle" | Cc"zy" | Cc"zyż" | Cc"hyba" | Cc"zyli" |
                               Jj"ednak" | Ii  | Ll"ub" | Ll"ecz" | Nn"iech" | Nn"iechaj" |
                               Nn"im" | Nn"iż" | Nn"uż" | Oo"raz" | Pp"onieważ" | Tt"edy" | Tt"rudno" |
                               Ss"koro" | Ww"ięc" | Zz"anim" Zz"atem" | Zz"aś" |
                               Pred | "powinno" )
                           ] ;

### Inflected singular forms of "by"
    Singular_by = 
                    ("bym" | "byś" | "by") ;

### Inflected plural forms of "by"
    Plural_by = 
                    ("byśmy" | "byście" | "by") ;

### All inflected forms of "by"
    Ending_by = ( Singular_by | Plural_by );

    Separate_by =
                       Particle_by
                          ("": " ") 
                              Ending_by ;

    export Rule1939_B =
                         Separate_by ;

## "Nie + modal verbs
The particle "nie" should be separated from modal verbs.
> Nietrzeba -> Nie trzeba
> niepotrzeba -> nie potrzeba
> Niepowinno -> Nie powinno

    SeparateModal = 
                   Nn"ie"
                       ("" : " ")
                                 (Pred | "powinno" | "należy") ;
                           
    export Rule1950 = 
                      SeparateModal ;


#### Rules that have not been motivated by Malinowski's work
## This list contains cores (without flexional endings) of words whose spelling evolved in a unique manner.
## First: words with double letters that have been simplified

## Double F
> officer -> oficer
    DoubleF = Optimize   [ (
                                 Kk("offein" : "ofein") |
                                 Oo("fficer" : "ficer") |
                                 Tt("aryff" : "aryf") 
                           )
                         ] ;

    export Rule1832_B = 
                        DoubleF
                           AnyString ;

## Double P
> Rapporta -> Raporty
> oppozycyjny -> opozycyjny
    DoubleP = Optimize [ (
                            Rr("apport" : "aport") |
                            Oo("ppozyc" : "pozyc")
                         )
                       ] ;

    export Rule1835 = 
                        DoubleP
                           AnyString ;

## Double M
> grammatyka -> gramatyka
    DoubleM = Optimize   [ (
                                 Gg("rammat" : "ramat" ) |
                                 Hh("omm" : "om") Vowel |
                                 Rr("ekommend" : "ekomend") 
                           )
                         ] ;

    export Rule1862_A = 
                        DoubleM
                           AnyString ;

## Double R
> korrespondować -> korespondować
    DoubleR = Optimize   [ (
                                 Gg("omorr" | "omor" ) |
                                 Kk("orrespond" : "orespond") |
                                 Tt("errytor" : "erytor")
                           )
                         ] ;

    export Rule1862_B = 
                        DoubleR
                           AnyString ; 

##Double S
>professor -> profesor
>interessa -> interesy

    DoubleS = Optimize [ (
                                 ( "Bessarab"  : "Besarab") |
                                 Dd("ygress" : "ygres") |
                                 Dd("yssyden" : "ysyden") |
                                 Ee("ss" : "s") "e" ("i" | "j") |
                                 Ii("nteress" : "nteres") |
                                 Kk("ass" : "as" ) |
                                 Kk("lass" : "las" ) |
                                 Kk("omiss" : "omis") |
                                 Mm("iss"("j"|"y") : "isj") |
                                 Pp("rass" : "ras")  |
                                 Pp("rofessor" : "rofesor") |
                                 Pp("russ" : "rus") |
                                 Rr("oss" : "os") (("ya": "ja") | "y" | "j") |
                                 Ss("trassbur" : "trasbur") 
                         )
                       ] ;

    export Rule1865 = 
                        DoubleS
                           AnyString ; 

##Double L
> illustracja -> ilustracja 
    DoubleL = Optimize   [ (
                                 Ii("llustr" : "lustr") |
                                 Kk("olleg" : "oleg") |
                                 Kk("ollend" : "olęd") 
                           )
                         ] ;

    export Rule1873 = 
                        DoubleL
                           AnyString ; 

## Double N
> tennisowy -> tenisowy 
    DoubleN = Optimize [ (
                                 Hh("anno"("v"|"w")"er" :"anower") |
                                 Tt("ennis" : "enis" )
                         )
                       ] ;

    export Rule1931 = 
                        DoubleN
                           AnyString ; 

## Double K
> Marokko -> Maroko
    DoubleK = Optimize   [ (
                                 Aa("kkomod" : "komod" ) |
                                 Aa("kkumul" : "kumul" ) |
                                 Mm("arokk" : "arok")   
                           )
                         ] ;

    export Rule1948 = 
                        DoubleK
                           AnyString ; 

## Rule "Osiągnienia"
The rule converts some gerunds ending with "nienie"

> osiągnienia > osiągnięcia
> osiągnień > osiągnięć
> zwolnienie -> zwolnienie

    GerundEnding = Optimize [
                               ( "e"| "a" | "u" | "em" | "om" | "ami" | "ach" ) 
                            ] ;
    Replace_ni_with_ci = 
                  ("nie": "nię")
                       ( ("ni" : "ci" ) GerundEnding | ("ń" : "ć") );

    export Rule1859 = 
                       ( NonEmptyString
                         Replace_ni_with_ci ) ;
    %ger
    

## Rules that change "x" into "gz"
> exam -> egzam
> exekucja -> egzekucja
> exema -> egzema
    Replace_x_with_gz = Optimize [ (
                                     Ee("xam" : "gzam") |
                                     Ee("xek" : "gzek") |
                                     Ee("xeg" : "gzeg") |
                                     Ee("xem" : "gzem") 
                                   )
                                 ] ;

    export Rule1860 = 
                        Replace_x_with_gz
                           AnyString ; 

## Rule PluralNounForms
The rule changes nominative plural forms of selected nouns. Requires Rule100_01 for the word: "examina". The rule does not use CDRewrite!
> argumenta -> argumenty
> examina -> egzaminy

## Nouns that used to be ended with 'a' in nominative plural; now they are ended with 'y'

    SpecialNoun = Optimize [
                             (
                               Aa"partament"  |
                               Aa"rgument"    |
                               Dd"epartament" |
                               Dd"epozyt"     |
                               Ee"gzamin"     |
                               Ff"undament"   |
                               Ii"nstrument"  |
                               Ii"nstynkt"    |
                               Ii"nteres"     |
                               Kk"onkur"      |
                               Kk"redyt"      |
                               Mm"oment"      |
                               Pp"aszport"    |
                               Pp"atent"      |
                               Pp"rodukt"     |
                               Pp"rojekt"     |
                               Pp"rotest"     |
                               Rr"aport"      |
                               Rr"ejestr"     |
                               Rr"ezultata"   |
                               Ss"krypt"      |
                               Tt"ransport"
                             )
                           ] ;

    PluralNounForms = 
                      SpecialNoun ("a" :"y" );

    export Rule1896_B = 
                       PluralNounForms ;

## Rules of the German origin
> fahryka -> fabryka
    GermanOrigin = Optimize [ (
                                Ff("ahryk" : "abryk" ) |
                                Ff("enyg" : "enig" ) |
                                Ff("rankfor" : "rankfur") |
                                Mm("alborg" : "albork") |
                                Mm("athias" : "atias")
                              )
                            ] ;

    export Rule1918 = 
                        GermanOrigin
                           AnyString ; 

## Rule UniqueEnding. 

The rule converts endings that have evolved in a unigue manner.
The rule does not use CDRewrite!
Word "egoizmie is not processed correctly becuase after operation of Rule100_03, Rule001_08, changes "i" into "j". This is caused by the fact that after the first modification the word "egoizmie" is not verified in the 'exception list'.
> kuracjuszów -> kuracjuszy
> monarchi -> monarchii
> epopeji > epopei
> ojczyznie -> ojczyźnie

    NounEnding = Optimize [ (
                                ("uszów" : "uszy") |
                                ("arch"("j"|"")"i" : "archii") |
                                ("eji" : "ei" ) |
                                ("y"|"i")("z":"ź")"nie" |
                                ("y"|"i")("ź":"z")"mie" 
                            )
                          ] ;

    export Rule1923 = 
                        NonEmptyString
                            NounEnding ;

## Rule UniquePrefix
The rule converts prefixes that have evolved in a unigue manner. The rule does not use CDRewrite! Test cases are strange to avoid exceptions.

>vicekontroler -> wicekontroler
>vicepremjerostwo -> wicepremierostwo
>contradmirał -> kontradmirał
    UniquePrefix = Optimize [
                              (
                                  ( ("V"|"W") : "W" ) |
                                  ( ("v"|"w") : "w" )
                              )
                                    "ice"
                                         (("-"|"") : "")|
                              (   
                                  ( ("C" | "K") : "K") | 
                                  ( ("c" | "k") : "k")
                              )
                                    "ontr"
                                         (("-"|"") : "") 
                            ] ;

    export Rule1987 = 
                         UniquePrefix                      
                              NonEmptyString ;

## Various observed phonetic changes

> juryzdykcją -> jurysdykcją
> hygiena -> higiena
    PhoneticChanges = Optimize [ (
                                 Bb("iblijote" : "ibliote") |
                                 Cc("entim" : "entym") |
                                 Cc("hrześcian" : "hrześcijan") |
                                 Cc("hrześciań" : "hrześcijań") |
                                 Dd("racaen" : "racen" ) |
                                 Ff("izyjo" : "izjo" ) |
                                 Ff("ranci" : "rancj" )  Vowel |
                                 Ff("rancuzk" : "rancus") |
                                 Hh("ygien" :  "igien")|
                                 Hh("ypot" : "ipot" ) |
                                 Hh("ypno" : "ipno" ) |
                                 Ii("nżenier" : "nżynier") |
                                 ( ("J":"G") | ("j" : "g") )("eo" : "eo") |
                                 ( ("J":"G") | ("j" : "g") )("enera" : "enera") |
                                 Jj("uryzdyk" : "urysdyk" ) |
                                 Kk("lijent" : "lient") |
                                 Kk("loset" : "lozet") |
                                 Kk("onsumc" : "onsumpc") |
                                 Ll("isbon" : "izbon" ) |
                                 Mm("arszant" : "arszand") | 
                                 Mm("a"("x"|"ks")"ymum" : "aksimum") |
                                 Mm("ater"("i"|"y"|"j"|"yj")"a"("l"|"ł") : "ateriał" ) |
                                 Mm("ilijon" : "ilion") |
                                 Oo("fenzyw" : "fensyw") |
                                 Pp("atryja" : "atria") |
                                 Pp("aznogc" : "aznokc") |
                                 Rr("edakci" : "edakcj") |
                                 Rr("egestr" : "ejestr") |
                                 Rr("oentgen" : "entgen" ) | 
                                 ( Dd("i": "y") | "") Ss("cont" : "kont" ) |
                                 Tt("ranzakc" : "ransakc") |
                                 Ww("szysk" : "szystk") |
                                 Zczc("ydowstw" : "ydostw")
                              )
                         ] ;

## Other changes (motivated grammtically?)
    OtherChanges = Optimize [ (
                                 Dd("okonywuj" : "okonuj" ) |
                                 Dd("owódzc" : "dowódc" ) |
                                 Dd("ruch" : "ruh" ) |                               
                                 Ff("estival" : "estiwal")|                        
                                 Hh("ong-"Kk"ong" : "Hongkong") |
                                 Ii("kognito" : "cognito" ) |
                                 Kk("ino-teatr" : "inoteatr") |
                                 Kk("ołomyji" : "ołomyi" ) |
                                 Oo("biedw" : "bydw") |
                                 Pp("asorzy" : "asoży") |
                                 Pp"atrz"( ("a":"y")"ł" | ("a":"e")"l" ) |
                                 Pp("en" : "ę")"dz"("el"|"l") |
                                 Pp"etersbur"("g" : "")"s" |
                                 Pp("etyt":"etit") |
                                 Pp("łutn" : "łótn" ) |
                                 Pp("ojedyńcz" : "ojedyncz" ) |
                                 Pp("osred" : "ośred" )  |
                                 Pp("rotokuł" : "rotokół") |
                                 Pp("rusiech" : "rusach" ) |
                                 Pp("rzedtym" : "rzedtem") |
                                 Pp("rzywódzc" : "rzywódc") |
                                 Pp("uhar" : "uchar") |                                
                                 Rr"oz"("s"|"")"trzygni" ("e" : "ę") (("n" : "c") | ("ń" : "ć"))  |
                                 Rr("oztrzygn" : "ozstrzygn") |
                                 Rr("ybołóstw" : "ybołówstw" )  |
                                 Rr("ządzc" : "ządc") |
                                 Rr("zeczypospolit" : "rzeczpospolit") |
                                 Ss("alvat" : "alwat") |
                                 Ss("pecial" : "pecjal") |
                                 ("s": "ws") "pół" |
                                 Ss("tósunk" : "tosunk" )|
                                 Ss("woji" : "woi") |
                                 Tt("eror" : "error") |
                                 Tt("łómacz" : "łumacz") |
                                 Uu("czenic" : "czennic" ) |
                                 Uu("niknien" : "niknięc" ) |
                                 Ww("skutku" : "skutek") |
                                 Ww("spomni"(("o"|"e") : "a")"n") |
                                 Ww("ykonywuj" : "ykonuj") 
                            )
                          ] ;
    
    UniqueWord = Optimize [ (
                              PhoneticChanges    |
                              OtherChanges 
                            )
                          ] ;

## Rule that normalizes unnique words if preceded by a prefix.
The rule converts words that have evolved in a unigue manner preceded by a prefix.
> wytłómaczy -> wytłumaczy
> zarządzca -> zarządca

## Selected prefixes that may precede verbs or nouns 
    GeneralPrefix = Optimize[
                               ( Dd"o" | Nn"a" | Nn"ad" | Nn"ade" | Oo | Oo"d" | Oo"de"| Pp"o" |
                                 Pp"od" | Pp"ode" | Pp"oza" | Pp"rzeciw" | Pp"rze" | Pp"rzede" |
                                 Pp"rzy"| Uu | Ww | Ww"e" | Ww"y" | Zz | Zz"a" | Zz"e"
                               )
                            ] ;
    export Rule2018_01 =
                      GeneralPrefix
                        UniqueWord
                           AnyString ; 

## Rule 'Puhar'
The rule converts words that have evolved in a unigue manner. The rule does not use CDRewrite!

> hygieniczny -> higieniczny
> dokonywuje -> dokonuje
> Frankforcie -> Frankfurcie
> illustracyi -> ilustracji
> materyjal -> materiał
> patrzali -> patrzeli
> Rossyanie -> Rosjanie
> puharom -> pucharom

    export Rule2018_02 = 
                        UniqueWord
                           AnyString ; 

## Rule 'Gdański'
The rule converts morphemes that have evolved in a unigue manner. The rule uses CDRewrite, becuase it may be repeated or combined with other rules.

    UniqueMorpheme = Optimize [
                                (
                                  ("nsk" : "ńsk") |
                                  (("s" : "ś") | ("S" : "Ś")) (("c" | "m" | "n" ) "i" | "pie")                            
                                )
                              ] ;

> Gdansk -> Gdańsk
> gdynski-> gdyński
> pismie -> piśmie
> posciel -> pościel

    export Rule2018_03 = Optimize[
           CDRewrite[ UniqueMorpheme,
                      "",
                      "",
                      Any* ] ];

## Rules for verb endings
> znaleść -> znaleźć
> powieść -> powieść

    VerbEnding = Optimize [ (
                                ("leść":"leźć") |
                                ("gryść":"gryźć") |
                                ("śdź" : "ść") |
                                ("najmywać" : "najmować" )
                              )
                            ] ;

    export Rule2018_04 = 
                        NonEmptyString
                            VerbEnding ;


## Rule "Przeznaczon"
The rule adds "y" to forms of verbs like "będziem", "zrobim". The rule is turned off for finite forms (będziem) as the composition takes too long.
> zrobim > zrobimy
> przyjdziem -> przyjdziemy

    Add_y_ToVerb = 
                  ("im": "imy") |
                  ("em" : "emy") ;

    export Rule2018_05 = 
                      (NonEmptyString
                        Add_y_ToVerb)
                            @VerbFin ;
    %verb&number=pl&person=pri&tense=fin
    

## Rule "Przeznaczon"
The rule adds "y" to participles "zadowolon" "przeznaczon"

> zadowolon > zadowolony
> przeznaczon -> przeznaczony
> karan -> karany

    Add_y_ToPpas = 
                  ("a" | "o")  ("n": "ny") ;

    export Rule2018_06 = 
                      (NonEmptyString
                         Add_y_ToPpas) ;
    %ppas

###################### END OF RULES ##########################

## Two technical "PostRules"

    Cleaning_y =
                    ("$y$" : "y" );

    export PostRuleFixY = Optimize[
           CDRewrite[ Cleaning_y,
                      "",
                      "",
                      Any* ] ];

    Cleaning_i =
                    ("$i$" : "i" );

    export PostRuleFixI = Optimize[
           CDRewrite[ Cleaning_i,
                      "",
                      "",
                      Any* ] ];

    Cleaning_j =
                    ("$j$" : "j" );

    export PostRuleFixJ = Optimize[
           CDRewrite[ Cleaning_j,
                      "",
                      "",
                      Any* ] ];



