#include "tests.hpp"

#include "lattice.hpp"
#include "annotation_item.hpp"
#include "annotation_item_manager.hpp"

#include "by_spaces_cutter.hpp"
#include "fake_lemmatizer.hpp"
#include "lemmatizer_annotator.hpp"
#include "layer_tag_collection.hpp"

BOOST_AUTO_TEST_SUITE( lattice )

void initAndTokenize_(Lattice& lattice, const std::string& paragraph, bool addSymbols=true);

BOOST_AUTO_TEST_CASE( lattice_simple ) {
    AnnotationItemManager aim;
    Lattice lattice(aim, "Ala ma kota");

    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());

    BOOST_CHECK_EQUAL(lattice.getAllText(), "Ala ma kota");

    Lattice::EdgesSortedBySourceIterator ei
        = lattice.edgesSortedBySource(lattice.getLayerTagManager().anyTag());
    BOOST_CHECK(ei.hasNext());
    Lattice::EdgeDescriptor firstEdge = ei.next();
    BOOST_CHECK_EQUAL(
        lattice.getEdgeBeginIndex(firstEdge),
        lattice.getVertexRawCharIndex(lattice.getFirstVertex())
    );
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(firstEdge), "'A");
    BOOST_CHECK(ei.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ei.next()), "'l");
    BOOST_CHECK(ei.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ei.next()), "'a");
    BOOST_CHECK(ei.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ei.next()), "' ");
    BOOST_CHECK(ei.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ei.next()), "'m");
    BOOST_CHECK(ei.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ei.next()), "'a");
    BOOST_CHECK(ei.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ei.next()), "' ");
    BOOST_CHECK(ei.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ei.next()), "'k");
    BOOST_CHECK(ei.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ei.next()), "'o");
    BOOST_CHECK(ei.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ei.next()), "'t");
    BOOST_CHECK(ei.hasNext());
    Lattice::EdgeDescriptor lastEdge = ei.next();
    BOOST_CHECK_EQUAL(
        lattice.getEdgeEndIndex(lastEdge),
        lattice.getVertexRawCharIndex(lattice.getLastVertex())
    );
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(lastEdge), "'a");
    BOOST_CHECK(!ei.hasNext());
    BOOST_CHECK_THROW(ei.next(), NoEdgeException);

    Lattice::VertexDescriptor pre_ala = lattice.getFirstVertex();
    Lattice::VertexDescriptor post_ala = lattice.getVertexForRawCharIndex(3);
    Lattice::VertexDescriptor pre_ma = lattice.getVertexForRawCharIndex(4);
    Lattice::VertexDescriptor post_ma = lattice.getVertexForRawCharIndex(6);
    Lattice::VertexDescriptor pre_kota = lattice.getVertexForRawCharIndex(7);
    Lattice::VertexDescriptor post_kota = lattice.getLastVertex();

    LayerTagCollection
        raw_tag = lattice.getLayerTagManager().createSingletonTagCollection("symbol");
    LayerTagCollection
        token_tag = lattice.getLayerTagManager().createSingletonTagCollectionWithLangCode(
            "token", "pl");

    LayerTagMask rawMask = lattice.getLayerTagManager().getMask(raw_tag);
    LayerTagMask tokenMask = lattice.getLayerTagManager().getMask(token_tag);

    AnnotationItem word_token("word");
    AnnotationItem blank_token("blank");

    {
        Lattice::EdgeSequence::Builder ala_builder(lattice);
        ala_builder.addEdge(lattice.firstOutEdge(
                                lattice.getVertexForRawCharIndex(0),
                                rawMask));
        ala_builder.addEdge(lattice.firstOutEdge(
                                lattice.getVertexForRawCharIndex(1),
                                rawMask));
        ala_builder.addEdge(lattice.firstOutEdge(
                                lattice.getVertexForRawCharIndex(2),
                                rawMask));

        lattice.addEdge(pre_ala, post_ala, word_token, token_tag, ala_builder.build());
    }

    {
        Lattice::EdgeSequence::Builder first_blank_builder(lattice);
        first_blank_builder.addEdge(lattice.firstOutEdge(
                                        lattice.getVertexForRawCharIndex(3),
                                        rawMask));
        lattice.addEdge(post_ala, pre_ma, blank_token, token_tag, first_blank_builder.build());
    }

    {
        Lattice::EdgeSequence::Builder ma_builder(lattice);
        ma_builder.addEdge(lattice.firstOutEdge(
                               lattice.getVertexForRawCharIndex(4),
                               rawMask));
        ma_builder.addEdge(lattice.firstOutEdge(
                               lattice.getVertexForRawCharIndex(5),
                               rawMask));
        lattice.addEdge(pre_ma, post_ma, word_token, token_tag, ma_builder.build());
    }

    {
        Lattice::EdgeSequence::Builder second_blank_builder(lattice);
        second_blank_builder.addEdge(lattice.firstOutEdge(
                                         lattice.getVertexForRawCharIndex(6),
                                         rawMask));
        lattice.addEdge(post_ma, pre_kota, blank_token, token_tag, second_blank_builder.build());
    }

    {
        Lattice::EdgeSequence::Builder kota_builder(lattice);
        kota_builder.addEdge(lattice.firstOutEdge(
                                 lattice.getVertexForRawCharIndex(7),
                                 rawMask));
        kota_builder.addEdge(lattice.firstOutEdge(
                                 lattice.getVertexForRawCharIndex(8),
                                 rawMask));
        kota_builder.addEdge(lattice.firstOutEdge(
                                 lattice.getVertexForRawCharIndex(9),
                                 rawMask));
        kota_builder.addEdge(lattice.firstOutEdge(
                                 lattice.getVertexForRawCharIndex(10),
                                 rawMask));
        lattice.addEdge(pre_kota, post_kota, word_token, token_tag, kota_builder.build());
    }

    // tests

    Lattice::EdgesSortedBySourceIterator tokenIter = lattice.edgesSortedBySource(tokenMask);
    BOOST_CHECK(tokenIter.hasNext());

    BOOST_CHECK_EQUAL(
        lattice.getAnnotationCategory(tokenIter.next()),
        word_token.getCategory()
        );
    BOOST_CHECK(tokenIter.hasNext());

    BOOST_CHECK_EQUAL(
        lattice.getAnnotationCategory(tokenIter.next()),
        blank_token.getCategory()
        );
    BOOST_CHECK(tokenIter.hasNext());

    BOOST_CHECK_EQUAL(
        lattice.getAnnotationCategory(tokenIter.next()),
        word_token.getCategory()
        );
    BOOST_CHECK(tokenIter.hasNext());

    BOOST_CHECK_EQUAL(
        lattice.getAnnotationCategory(tokenIter.next()),
        blank_token.getCategory()
        );
    BOOST_CHECK(tokenIter.hasNext());

    BOOST_CHECK_EQUAL(
        lattice.getAnnotationCategory(tokenIter.next()),
        word_token.getCategory()
        );
    BOOST_CHECK(!tokenIter.hasNext());

    BOOST_CHECK_THROW(
        lattice.firstInEdge(lattice.getFirstVertex(), lattice.getLayerTagManager().anyTag()),
        NoEdgeException
    );
    BOOST_CHECK_THROW(
        lattice.firstOutEdge(lattice.getLastVertex(), lattice.getLayerTagManager().anyTag()),
        NoEdgeException
    );

    BOOST_CHECK_THROW(
        lattice.getVertexForRawCharIndex(lattice.getVertexRawCharIndex(lattice.getFirstVertex())-1),
        NoVertexException
    );
    BOOST_CHECK_THROW(
        lattice.getVertexForRawCharIndex(lattice.getVertexRawCharIndex(lattice.getLastVertex())+1),
        NoVertexException
    );

}

BOOST_AUTO_TEST_CASE( vertex_iterator ) {
    AnnotationItemManager aim;
    Lattice lattice(aim);
    lattice.appendStringWithSymbols("żeś");

    Lattice::VertexIterator iter(lattice);

    size_t count = 0;
    while (iter.hasNext()) {
        iter.next();
        ++count;
    }

    BOOST_CHECK_EQUAL(count, 4U);
}

BOOST_AUTO_TEST_CASE( get_path ) {
    AnnotationItemManager aim;
    Lattice lattice(aim);
    lattice.appendStringWithSymbols("ćma zielona");
    Lattice::VertexDescriptor markupBegin = lattice.getLastVertex();
    lattice.appendString("<br>");
    Lattice::VertexDescriptor markupEnd = lattice.getLastVertex();
    lattice.appendStringWithSymbols("mucha");
    lattice.appendString("<br>");

    AnnotationItem aiBlank("B");
    LayerTagCollection tagToken
        = lattice.getLayerTagManager().createSingletonTagCollection("token");
    lattice.addEdge(markupBegin, markupEnd, aiBlank, tagToken, Lattice::EdgeSequence());

    LayerTagMask symbolMask = lattice.getLayerTagManager().getMask("symbol");

    Lattice::VertexDescriptor vertex = lattice.getFirstVertex();

    Lattice::EdgeSequence sequence = lattice.getPath(vertex, symbolMask);
    BOOST_CHECK_EQUAL(lattice.getSequenceText(sequence), "ćma zielona");

    Lattice::InOutEdgesIterator iter = lattice.outEdges(vertex, symbolMask);
    BOOST_CHECK(!iter.hasNext());

    vertex = lattice.getFirstVertex();

    Lattice::EdgeSequence sequenceWithBlank = lattice.getPathSkippingBlanks(vertex, symbolMask);
    BOOST_CHECK_EQUAL(lattice.getSequenceText(sequenceWithBlank), "ćma zielona<br>mucha");

    Lattice::InOutEdgesIterator iter2 = lattice.allOutEdges(vertex);
    while (iter2.hasNext()) {
        BOOST_CHECK(!lattice.isBlank(iter2.next()));
    }

    BOOST_CHECK_EQUAL(sequence.size(lattice), 11U);
}

BOOST_AUTO_TEST_CASE( cutter ) {
    AnnotationItemManager aim;
    Lattice lattice(aim);
    initAndTokenize_(lattice, "szybki zielony rower");

    LayerTagMask tokenMask = lattice.getLayerTagManager().getMaskWithLangCode(
        "token", "pl");
    Lattice::EdgesSortedBySourceIterator tokenIter = lattice.edgesSortedBySource(tokenMask);

    {
        BOOST_CHECK(tokenIter.hasNext());
        AnnotationItem item = lattice.getEdgeAnnotationItem(tokenIter.next());
        BOOST_CHECK_EQUAL(item.getText(), "szybki");
        BOOST_CHECK_EQUAL(item.getCategory(), "word");
    }

    {
        BOOST_CHECK(tokenIter.hasNext());
        AnnotationItem item = lattice.getEdgeAnnotationItem(tokenIter.next());
        BOOST_CHECK_EQUAL(item.getText(), " ");
        BOOST_CHECK_EQUAL(item.getCategory(), "blank");
    }

    {
        BOOST_CHECK(tokenIter.hasNext());
        AnnotationItem item = lattice.getEdgeAnnotationItem(tokenIter.next());
        BOOST_CHECK_EQUAL(item.getText(), "zielony");
        BOOST_CHECK_EQUAL(item.getCategory(), "word");
    }

    {
        BOOST_CHECK(tokenIter.hasNext());
        AnnotationItem item = lattice.getEdgeAnnotationItem(tokenIter.next());
        BOOST_CHECK_EQUAL(item.getText(), " ");
        BOOST_CHECK_EQUAL(item.getCategory(), "blank");
    }

    {
        BOOST_CHECK(tokenIter.hasNext());
        AnnotationItem item = lattice.getEdgeAnnotationItem(tokenIter.next());
        BOOST_CHECK_EQUAL(item.getText(), "rower");
        BOOST_CHECK_EQUAL(item.getCategory(), "word");
    }

    BOOST_CHECK(!tokenIter.hasNext());
}

BOOST_AUTO_TEST_CASE( cutter_on_no_symbols ) {
    AnnotationItemManager aim;
    Lattice lattice(aim);
    initAndTokenize_(lattice, "<a>", false);

    LayerTagMask tokenMask = lattice.getLayerTagManager().getMask("token");

    Lattice::EdgesSortedBySourceIterator tokenIter = lattice.edgesSortedBySource(tokenMask);

    BOOST_CHECK(!tokenIter.hasNext());
}

BOOST_AUTO_TEST_CASE( lemmatizer ) {
    AnnotationItemManager aim;
    Lattice lattice(aim);
    initAndTokenize_(lattice, "prowokacjami");

    boost::program_options::variables_map noOptions;
    LemmatizerAnnotator<FakeLemmatizer> annotator(noOptions);

    annotator.annotate(lattice);

    // now checking

    {
        LayerTagMask lemmaMask_ = lattice.getLayerTagManager().getMask("lexeme");
        Lattice::EdgesSortedByTargetIterator lemmaIter = lattice.edgesSortedByTarget(lemmaMask_);

        BOOST_CHECK(lemmaIter.hasNext());
        Lattice::EdgeDescriptor prowokacjamiLemma = lemmaIter.next();
        AnnotationItem prowokacjamiItem = lattice.getEdgeAnnotationItem(prowokacjamiLemma);

        BOOST_CHECK_EQUAL(prowokacjamiItem.getCategory(), "R");
        BOOST_CHECK_EQUAL(prowokacjamiItem.getText(), "prowokacja_R");

        BOOST_CHECK_EQUAL(
            lattice.getVertexRawCharIndex(lattice.getEdgeSource(prowokacjamiLemma)),
            0U);

        BOOST_CHECK_EQUAL(
            lattice.getVertexRawCharIndex(lattice.getEdgeTarget(prowokacjamiLemma)),
            12U);

        BOOST_CHECK(!lemmaIter.hasNext());
    }

    {
        LayerTagMask formMask_ = lattice.getLayerTagManager().getMask("form");
        Lattice::EdgesSortedByTargetIterator formIter = lattice.edgesSortedByTarget(formMask_);

        BOOST_CHECK(formIter.hasNext());
        Lattice::EdgeDescriptor prowokacjamiForm = formIter.next();
        AnnotationItem prowokacjamiItem = lattice.getEdgeAnnotationItem(prowokacjamiForm);

        BOOST_CHECK_EQUAL(prowokacjamiItem.getCategory(), "R");
        BOOST_CHECK_EQUAL(prowokacjamiItem.getText(), "prowokacja_R");

        BOOST_CHECK(!formIter.hasNext());
    }
}


BOOST_AUTO_TEST_CASE( simple_get_descendents ) {
    AnnotationItemManager aim;
    Lattice lattice(aim);
    initAndTokenize_(lattice, "prowokacjami");

    boost::program_options::variables_map noOptions;
    LemmatizerAnnotator<FakeLemmatizer> annotator(noOptions);

    annotator.annotate(lattice);

    // now checking

    LayerTagMask lemmaMask_ = lattice.getLayerTagManager().getMask("lexeme");
    Lattice::EdgesSortedByTargetIterator lemmaIter = lattice.edgesSortedByTarget(lemmaMask_);

    BOOST_REQUIRE(lemmaIter.hasNext());
    Lattice::EdgeDescriptor prowokacjamiLemma = lemmaIter.next();

    LayerTagMask formMask_ = lattice.getLayerTagManager().getMask("form");

    std::vector<Lattice::EdgeDescriptor> forms = lattice.getChildren(
        prowokacjamiLemma,
        formMask_);

    BOOST_REQUIRE_EQUAL(forms.size(), 1);

    std::vector<Lattice::EdgeDescriptor> formsOnForms = lattice.getChildren(
        forms[0],
        formMask_);

    BOOST_REQUIRE(formsOnForms.empty());
}


BOOST_AUTO_TEST_CASE( variant_edges ) {
    //preparing lattice
    AnnotationItemManager aim;
    Lattice lattice(aim, "ananas");
    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());
    LayerTagCollection raw_tag
        = lattice.getLayerTagManager().createSingletonTagCollection("symbol");
    LayerTagCollection token_tag
        = lattice.getLayerTagManager().createSingletonTagCollectionWithLangCode(
            "token", "pl");
    LayerTagCollection lemma_tag
        = lattice.getLayerTagManager().createSingletonTagCollection("lemma");
    LayerTagMask rawMask = lattice.getLayerTagManager().getMask(raw_tag);
    LayerTagMask tokenMask = lattice.getLayerTagManager().getMask(token_tag);
    LayerTagMask lemmaMask = lattice.getLayerTagManager().getMask(lemma_tag);

    Lattice::VertexDescriptor pre_ananas = lattice.getFirstVertex();
    Lattice::VertexDescriptor post_ananas = lattice.getVertexForRawCharIndex(6);

    AnnotationItem word_token("word");
    AnnotationItem blank_token("blank");

    Lattice::EdgeSequence::Builder ananas_builder(lattice);
    for (int i = 0; i < 6; i ++) {
        ananas_builder.addEdge(lattice.firstOutEdge(
                                   lattice.getVertexForRawCharIndex(i),
                                   rawMask
                                   ));
    }
    lattice.addEdge(pre_ananas, post_ananas, word_token, token_tag, ananas_builder.build());

    Lattice::EdgeSequence::Builder ananas_lemma_builder(lattice);
    ananas_lemma_builder.addEdge(
        lattice.firstOutEdge(lattice.getVertexForRawCharIndex(0), tokenMask));
    AnnotationItem ai_ananas_sg("ananas"); // singular masculinie noun variant
    aim.setValue(ai_ananas_sg, "base", "ananas");
    aim.setValue(ai_ananas_sg, "morphology", "subst:sg:m");
    aim.setValue(ai_ananas_sg, "discard", "0");
    lattice.addEdge(pre_ananas, post_ananas, ai_ananas_sg, lemma_tag, ananas_lemma_builder.build());

    AnnotationItem ai_ananas_pl("ananas"); // plural masculine noun variant
    aim.setValue(ai_ananas_pl, "base", "ananas");
    aim.setValue(ai_ananas_pl, "morphology", "subst:pl:m");
    aim.setValue(ai_ananas_pl, "discard", "0");
    lattice.addEdge(pre_ananas, post_ananas, ai_ananas_pl, lemma_tag, ananas_lemma_builder.build());

    Lattice::EdgeDescriptor edge;
    Lattice::EdgesSortedBySourceIterator tokenIter = lattice.edgesSortedBySource(lemmaMask);
    BOOST_CHECK(tokenIter.hasNext());
    edge = tokenIter.next();
    BOOST_CHECK_EQUAL(
        lattice.getAnnotationCategory(edge),
        ai_ananas_sg.getCategory()
        );
    std::list< std::pair<std::string, std::string> > av
        = aim.getValues(lattice.getEdgeAnnotationItem(edge));
    std::list< std::pair<std::string, std::string> >::iterator avi = av.begin();
    BOOST_CHECK_EQUAL((*avi).first, "base");
    BOOST_CHECK_EQUAL((*avi).second, "ananas");
    ++avi;
    BOOST_CHECK_EQUAL((*avi).first, "morphology");
    BOOST_CHECK_EQUAL((*avi).second, "subst:sg:m");
    ++avi;
    BOOST_CHECK(avi != av.end());
    BOOST_CHECK_EQUAL((*avi).first, "discard");
    BOOST_CHECK_EQUAL((*avi).second, "0");
    ++avi;
    BOOST_CHECK(avi == av.end());
    BOOST_CHECK(tokenIter.hasNext());
    edge = tokenIter.next();
    BOOST_CHECK_EQUAL(
        lattice.getAnnotationCategory(edge),
        ai_ananas_pl.getCategory()
        );
    av = aim.getValues(lattice.getEdgeAnnotationItem(edge));
    avi = av.begin();
    BOOST_CHECK_EQUAL((*avi).first, "base");
    BOOST_CHECK_EQUAL((*avi).second, "ananas");
    ++avi;
    BOOST_CHECK_EQUAL((*avi).first, "morphology");
    BOOST_CHECK_EQUAL((*avi).second, "subst:pl:m");
    ++avi;
    BOOST_CHECK(avi != av.end());
    BOOST_CHECK_EQUAL((*avi).first, "discard");
    BOOST_CHECK_EQUAL((*avi).second, "0");
    ++avi;
    BOOST_CHECK(avi == av.end());
    BOOST_CHECK(!tokenIter.hasNext());

}

BOOST_AUTO_TEST_CASE( edges_layer_tags ) {
    //preparing lattice
    AnnotationItemManager aim;
    Lattice lattice(aim, "ananas");
    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());
    LayerTagCollection raw_tag
        = lattice.getLayerTagManager().createSingletonTagCollection("symbol");
    LayerTagCollection token_tag
        = lattice.getLayerTagManager().createSingletonTagCollectionWithLangCode(
            "token", "pl");
    LayerTagMask rawMask = lattice.getLayerTagManager().getMask(raw_tag);
    LayerTagMask tokenMask = lattice.getLayerTagManager().getMask(token_tag);

    Lattice::VertexDescriptor pre_ananas = lattice.getFirstVertex();
    Lattice::VertexDescriptor post_ananas = lattice.getVertexForRawCharIndex(6);

    AnnotationItem word_token("word");
    AnnotationItem blank_token("blank");

    Lattice::EdgeSequence::Builder ananas_builder(lattice);
    for (int i = 0; i < 6; i ++) {
        ananas_builder.addEdge(lattice.firstOutEdge(
                                   lattice.getVertexForRawCharIndex(i),
                                   rawMask
                                   ));
    }
    lattice.addEdge(pre_ananas, post_ananas, word_token, token_tag, ananas_builder.build());

    Lattice::EdgeDescriptor edge;
    Lattice::EdgesSortedBySourceIterator rawIter = lattice.edgesSortedBySource(rawMask);
    BOOST_CHECK(rawIter.hasNext());
    edge = rawIter.next();
    std::list<std::string> tagNames
        = lattice.getLayerTagManager().getTagNames(lattice.getEdgeLayerTags(edge));
    BOOST_CHECK_EQUAL(tagNames.size(), (size_t) 1);
    std::list<std::string>::iterator tni = tagNames.begin();
    BOOST_CHECK_EQUAL(*tni, "symbol");
    ++tni;
    BOOST_CHECK(tni == tagNames.end());

    Lattice::EdgesSortedBySourceIterator tokenIter = lattice.edgesSortedBySource(tokenMask);
    BOOST_CHECK(tokenIter.hasNext());
    edge = tokenIter.next();
    tagNames = lattice.getLayerTagManager().getTagNames(lattice.getEdgeLayerTags(edge));
    BOOST_CHECK_EQUAL(tagNames.size(), (size_t) 2);
    tni = tagNames.begin();
    BOOST_CHECK_EQUAL(*tni, LayerTagManager::getLanguageTag("pl"));
    ++tni;
    BOOST_CHECK_EQUAL(*tni, "token");
    ++tni;
    BOOST_CHECK(tni == tagNames.end());
}

BOOST_AUTO_TEST_CASE( edges_tags_combining ) {
    //preparing lattice
    AnnotationItemManager aim;
    Lattice lattice(aim, "ananas");
    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());
    LayerTagCollection raw_tag
        = lattice.getLayerTagManager().createSingletonTagCollection("symbol");
    LayerTagCollection token_tag
        = lattice.getLayerTagManager().createSingletonTagCollection("token");
    LayerTagCollection lemma_tag
        = lattice.getLayerTagManager().createSingletonTagCollection("lemma");
    LayerTagMask rawMask = lattice.getLayerTagManager().getMask(raw_tag);
    LayerTagMask tokenMask = lattice.getLayerTagManager().getMask(token_tag);
    LayerTagMask lemmaMask = lattice.getLayerTagManager().getMask(lemma_tag);

    std::list<std::string> tokenOrLemmaMaskAsStrings
        = boost::assign::list_of
        (std::string("token"))
        (std::string("lemma"));

    LayerTagMask tokenOrLemmaMask =
        lattice.getLayerTagManager().getMask(tokenOrLemmaMaskAsStrings);

    Lattice::VertexDescriptor pre_ananas = lattice.getFirstVertex();
    Lattice::VertexDescriptor post_ananas = lattice.getVertexForRawCharIndex(6);

    AnnotationItem word_token("word");
    AnnotationItem blank_token("blank");

    Lattice::EdgeSequence::Builder ananas_builder(lattice);
    for (int i = 0; i < 6; i ++) {
        ananas_builder.addEdge(lattice.firstOutEdge(
                                   lattice.getVertexForRawCharIndex(i),
                                   rawMask
                                   ));
    }
    lattice.addEdge(pre_ananas, post_ananas, word_token, token_tag, ananas_builder.build());
    lattice.addEdge(pre_ananas, post_ananas, word_token, lemma_tag, ananas_builder.build());

    Lattice::EdgeDescriptor edge;
    Lattice::EdgesSortedBySourceIterator tokenOrLemmaIter
        = lattice.edgesSortedBySource(tokenOrLemmaMask);

    BOOST_CHECK(tokenOrLemmaIter.hasNext());
    edge = tokenOrLemmaIter.next();
    std::list<std::string> tagNames
        = lattice.getLayerTagManager().getTagNames(lattice.getEdgeLayerTags(edge));
    BOOST_CHECK_EQUAL(tagNames.size(), (size_t) 2);
    std::list<std::string>::iterator tni = tagNames.begin();
    BOOST_CHECK_EQUAL(*tni, "lemma");
    ++tni;
    BOOST_CHECK(tni != tagNames.end());
    BOOST_CHECK_EQUAL(*tni, "token");
}

BOOST_AUTO_TEST_CASE( edges_scores_combining ) {
    //preparing lattice
    AnnotationItemManager aim;
    Lattice lattice(aim, "ananas");
    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());
    LayerTagCollection raw_tag
        = lattice.getLayerTagManager().createSingletonTagCollection("symbol");
    LayerTagCollection token_tag
        = lattice.getLayerTagManager().createSingletonTagCollectionWithLangCode(
            "token", "pl");
    LayerTagMask rawMask = lattice.getLayerTagManager().getMask(raw_tag);
    LayerTagMask tokenMask = lattice.getLayerTagManager().getMask(token_tag);

    Lattice::VertexDescriptor pre_ananas = lattice.getFirstVertex();
    Lattice::VertexDescriptor post_ananas = lattice.getVertexForRawCharIndex(6);

    AnnotationItem word_token("word");
    AnnotationItem blank_token("blank");

    Lattice::EdgeSequence::Builder ananas_builder(lattice);
    for (int i = 0; i < 6; i ++) {
        ananas_builder.addEdge(lattice.firstOutEdge(
                                   lattice.getVertexForRawCharIndex(i),
                                   rawMask
                                   ));
    }

    lattice.addEdge(pre_ananas, post_ananas, word_token, token_tag, ananas_builder.build(), -8.0);
    Lattice::EdgesSortedBySourceIterator tokenIter1(lattice.edgesSortedBySource(tokenMask));
    BOOST_CHECK(tokenIter1.hasNext());
    BOOST_CHECK_EQUAL(lattice.getEdgeScore(tokenIter1.next()), -8.0);

    lattice.addEdge(pre_ananas, post_ananas, word_token, token_tag, ananas_builder.build(), -2.0);
    Lattice::EdgesSortedBySourceIterator tokenIter2(lattice.edgesSortedBySource(tokenMask));
    BOOST_CHECK(tokenIter2.hasNext());
    BOOST_CHECK_EQUAL(lattice.getEdgeScore(tokenIter2.next()), -2.0);

    lattice.addEdge(pre_ananas, post_ananas, word_token, token_tag, ananas_builder.build(), -4.0);
    Lattice::EdgesSortedBySourceIterator tokenIter3(lattice.edgesSortedBySource(tokenMask));
    BOOST_CHECK(tokenIter3.hasNext());
    BOOST_CHECK_EQUAL(lattice.getEdgeScore(tokenIter3.next()), -2.0);
}

BOOST_AUTO_TEST_CASE( loose_vertices ) {
    //preparing lattice
    AnnotationItemManager aim;
    Lattice lattice(aim, "abc");
    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());
    LayerTagCollection token_tag
        = lattice.getLayerTagManager().createSingletonTagCollection("token");

    LayerTagMask tokenMask = lattice.getLayerTagManager().getMask(token_tag);

    Lattice::VertexDescriptor vertexPre = lattice.getVertexForRawCharIndex(1);
    Lattice::VertexDescriptor vertexPost = lattice.getVertexForRawCharIndex(2);
    Lattice::VertexDescriptor vertexLoose = lattice.addLooseVertex();

    BOOST_CHECK_EQUAL(lattice.getLooseVertexIndex(vertexLoose), 0);
    BOOST_CHECK_THROW(lattice.getLooseVertexIndex(vertexPre), WrongVertexException);

    AnnotationItem tokenX("x");
    AnnotationItem tokenY("y");

    lattice.addEdge(vertexPre, vertexLoose, tokenX, token_tag);

    Lattice::InOutEdgesIterator eiPreOut = lattice.outEdges(vertexPre, tokenMask);
    BOOST_CHECK(eiPreOut.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(eiPreOut.next()), "x");

    Lattice::InOutEdgesIterator eiLooseIn = lattice.inEdges(vertexLoose, tokenMask);
    BOOST_CHECK(eiLooseIn.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(eiLooseIn.next()), "x");

    Lattice::EdgeDescriptor edgeLoose = lattice.addEdge(vertexLoose, vertexPost, tokenY, token_tag);

    Lattice::InOutEdgesIterator eiLooseOut = lattice.outEdges(vertexLoose, tokenMask);
    BOOST_CHECK(eiLooseOut.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(eiLooseOut.next()), "y");

    Lattice::InOutEdgesIterator eiPostIn = lattice.inEdges(vertexPost, tokenMask);
    BOOST_CHECK(eiPostIn.hasNext());
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(eiPostIn.next()), "y");

    Lattice::VertexDescriptor vertexLoose2 = lattice.addLooseVertex();
    BOOST_CHECK_THROW(lattice.addSymbols(vertexPre, vertexLoose), WrongVertexException);
    BOOST_CHECK_THROW(lattice.addSymbols(vertexLoose, vertexPost), WrongVertexException);
    BOOST_CHECK_THROW(lattice.addSymbols(vertexLoose, vertexLoose2), WrongVertexException);
    BOOST_CHECK_THROW(lattice.getEdgeLength(edgeLoose), WrongVertexException);
}

BOOST_AUTO_TEST_CASE( vertex_iterator_advanced ) {
    //preparing lattice
    AnnotationItemManager aim;
    Lattice lattice(aim, "abc");
    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());
    LayerTagCollection token_tag
        = lattice.getLayerTagManager().createSingletonTagCollection("token");
    LayerTagMask tokenMask = lattice.getLayerTagManager().getMask(token_tag);

    Lattice::VertexDescriptor vertexPre = lattice.getVertexForRawCharIndex(1);
    Lattice::VertexDescriptor vertexPost = lattice.getVertexForRawCharIndex(2);
    Lattice::VertexDescriptor vertexLoose = lattice.addLooseVertex();

    AnnotationItem tokenX("x");
    AnnotationItem tokenY("y");

    lattice.addEdge(vertexPre, vertexLoose, tokenX, token_tag);

    lattice.addEdge(vertexLoose, vertexPost, tokenY, token_tag);

    Lattice::VertexIterator iter(lattice);
    BOOST_CHECK(iter.hasNext());
    BOOST_CHECK_EQUAL(iter.next(), lattice.getFirstVertex());
    BOOST_CHECK(iter.hasNext());
    BOOST_CHECK_EQUAL(iter.next(), vertexPre);
    BOOST_CHECK(iter.hasNext());
    BOOST_CHECK_EQUAL(iter.next(), vertexLoose);
    BOOST_CHECK(iter.hasNext());
    BOOST_CHECK_EQUAL(iter.next(), vertexPost);
    BOOST_CHECK(iter.hasNext());
    BOOST_CHECK_EQUAL(iter.next(), lattice.getLastVertex());
    BOOST_CHECK(!iter.hasNext());
}

BOOST_AUTO_TEST_CASE( correction_erase ) {
    //preparing lattice
    AnnotationItemManager aim;
    Lattice lattice(aim, "cear");
    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());

    lattice.correctionErase(
        lattice.getVertexForRawCharIndex(1),
        lattice.getVertexForRawCharIndex(2)
        );

    LayerTagMask rawMask = lattice.getLayerTagManager().getMask(
        lattice.getLayerTagManager().createSingletonTagCollection("symbol")
        );

    Lattice::VertexDescriptor vd = lattice.getFirstVertex();
    Lattice::InOutEdgesIterator ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    Lattice::EdgeDescriptor ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'c") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'a") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'r") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }

    BOOST_CHECK_THROW(
        lattice.correctionErase(lattice.getFirstVertex(), lattice.getLastVertex()),
        WrongVertexException
    );
}

BOOST_AUTO_TEST_CASE( correction_insert ) {
    //preparing lattice
    AnnotationItemManager aim;
    Lattice lattice(aim, "cear");
    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());

    lattice.correctionInsert(
        lattice.getVertexForRawCharIndex(2),
        "z"
        );

    LayerTagMask rawMask = lattice.getLayerTagManager().getMask(
        lattice.getLayerTagManager().createSingletonTagCollection("symbol")
        );

    Lattice::VertexDescriptor vd = lattice.getFirstVertex();
    Lattice::InOutEdgesIterator ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    Lattice::EdgeDescriptor ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'c") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'e") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'z") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'a") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'r") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
}

BOOST_AUTO_TEST_CASE( correction_replace ) {
    //preparing lattice
    AnnotationItemManager aim;
    Lattice lattice(aim, "cear");
    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());

    lattice.correctionReplace(
        lattice.getVertexForRawCharIndex(1),
        lattice.getVertexForRawCharIndex(2),
        "z"
        );

    LayerTagMask rawMask = lattice.getLayerTagManager().getMask(
        lattice.getLayerTagManager().createSingletonTagCollection("symbol")
        );

    Lattice::VertexDescriptor vd = lattice.getFirstVertex();
    Lattice::InOutEdgesIterator ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    Lattice::EdgeDescriptor ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'c") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'z") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'a") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'r") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }

    BOOST_CHECK_THROW(
        lattice.correctionReplace(lattice.getFirstVertex(), lattice.getLastVertex(), "x"),
        WrongVertexException
    );
}

BOOST_AUTO_TEST_CASE( correction_replace_advanced ) {
    //preparing lattice
    AnnotationItemManager aim;
    Lattice lattice(aim, "cear");
    lattice.addSymbols(lattice.getFirstVertex(), lattice.getLastVertex());

    lattice.correctionReplace(
        lattice.getFirstVertex(),
        lattice.getLastVertex(),
        "czar"
        );

    LayerTagMask rawMask = lattice.getLayerTagManager().getMask(
        lattice.getLayerTagManager().createSingletonTagCollection("symbol")
        );

    Lattice::VertexDescriptor vd = lattice.getFirstVertex();
    Lattice::InOutEdgesIterator ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    Lattice::EdgeDescriptor ed = ei.next();
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ed), "'c");

    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    while (lattice.getAnnotationCategory(ed) != "'z") {
        BOOST_CHECK(ei.hasNext());
        ed = ei.next();
    }
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ed), "'z");

    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ed), "'a");

    vd = lattice.getEdgeTarget(ed);
    ei = lattice.outEdges(vd, rawMask);
    BOOST_CHECK(ei.hasNext());
    ed = ei.next();
    BOOST_CHECK_EQUAL(lattice.getAnnotationCategory(ed), "'r");
}

void initAndTokenize_(Lattice& lattice, const std::string& paragraph, bool addSymbols) {

    if (addSymbols)
        lattice.appendStringWithSymbols(paragraph);

    LayerTagCollection textTags(
        lattice.getLayerTagManager().createSingletonTagCollectionWithLangCode("text", "pl"));
    AnnotationItem item("TEXT", paragraph);
    try {
        lattice.addEdge(lattice.getFirstVertex(), lattice.getLastVertex(), item, textTags);
    } catch (LoopEdgeException) { }

    BySpacesCutter cutter;

    LayerTagMask symbolMask = lattice.getLayerTagManager().getMask("symbol");
    LayerTagMask textMask = lattice.getLayerTagManager().getMaskWithLangCode(
        "text", "pl");

    lattice.runCutter(cutter, symbolMask, textMask);
}

BOOST_AUTO_TEST_CASE( discard ) {
    AnnotationItemManager aim;
    Lattice lattice(aim);

    {
        initAndTokenize_(lattice, "Ala ma kota");

        LayerTagMask tokenMask = lattice.getLayerTagManager().getMask("token");
        Lattice::EdgesSortedBySourceIterator tokenIter = lattice.edgesSortedBySource(tokenMask);

        BOOST_CHECK(tokenIter.hasNext());
        tokenIter.next();
        BOOST_CHECK(tokenIter.hasNext());
        tokenIter.next();

        Lattice::EdgeDescriptor maEdge = tokenIter.next();
        AnnotationItem item = lattice.getEdgeAnnotationItem(maEdge);
        BOOST_CHECK_EQUAL(item.getText(), "ma");

        lattice.discard(maEdge);
    }

    {
        LayerTagMask discardedMask = lattice.getLayerTagManager().getMask("discarded");
        Lattice::EdgesSortedBySourceIterator discardedIter
            = lattice.edgesSortedBySource(discardedMask);

        BOOST_CHECK(discardedIter.hasNext());
        Lattice::EdgeDescriptor discardedEdge = discardedIter.next();
        AnnotationItem discardedItem = lattice.getEdgeAnnotationItem(discardedEdge);
        BOOST_CHECK_EQUAL(discardedItem.getText(), "ma");
        BOOST_CHECK_EQUAL(discardedItem.getCategory(), "word");
        BOOST_CHECK(!discardedIter.hasNext());
    }
}

BOOST_AUTO_TEST_CASE( edge_self_reference ) {
    AnnotationItemManager aim;
    Lattice lattice(aim, "a");
    Lattice::VertexDescriptor from = lattice.getFirstVertex();
    Lattice::VertexDescriptor to = lattice.getLastVertex();
    AnnotationItem item("item");
    LayerTagCollection tags(lattice.getLayerTagManager().createSingletonTagCollection("tag"));
    Lattice::EdgeDescriptor edge = lattice.addEdge(from, to, item, tags);

    Lattice::EdgeSequence::Builder builder(lattice);
    builder.addEdge(edge);

    BOOST_CHECK_THROW(
        lattice.addEdge(from, to, item, tags, builder.build()),
        EdgeSelfReferenceException
    );

    BOOST_CHECK_THROW(
        lattice.addPartitionToEdge(edge, tags, builder.build()),
        EdgeSelfReferenceException
    );

}

BOOST_AUTO_TEST_CASE( reversed_edges ) {
    AnnotationItemManager aim;
    Lattice lattice(aim, "abcd");
    Lattice::VertexDescriptor from = lattice.getVertexForRawCharIndex(3);
    Lattice::VertexDescriptor to = lattice.getVertexForRawCharIndex(1);
    AnnotationItem item("item");
    LayerTagCollection tags(lattice.getLayerTagManager().createSingletonTagCollection("tag"));

    BOOST_CHECK_THROW(
        lattice.addEdge(from, to, item, tags),
        ReversedEdgeException
    );
}


BOOST_AUTO_TEST_CASE( loop_edges ) {
    AnnotationItemManager aim;
    Lattice lattice(aim, "ab");
    Lattice::VertexDescriptor vertex = lattice.getVertexForRawCharIndex(1);
    AnnotationItem item("item");
    LayerTagCollection tags(lattice.getLayerTagManager().createSingletonTagCollection("tag"));

    BOOST_CHECK_THROW(
        lattice.addEdge(vertex, vertex, item, tags),
        LoopEdgeException
    );
}


BOOST_AUTO_TEST_CASE( lattice_vertices ) {
    AnnotationItemManager aim;
    Lattice lattice(aim);
    BOOST_CHECK_EQUAL(lattice.countAllVertices(), 1);

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, false);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 1);
    }

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, true);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 0);
    }

    lattice.appendString(std::string(3, 'a'));
    BOOST_CHECK_EQUAL(lattice.countAllVertices(), 4);

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, false);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 4);
    }

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, true);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 0);
    }

    lattice.appendStringWithSymbols(std::string(5, 'a'));
    BOOST_CHECK_EQUAL(lattice.countAllVertices(), 9);

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, false);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 9);
    }

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, true);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 6);
    }

    lattice.addLooseVertex();
    BOOST_CHECK_EQUAL(lattice.countAllVertices(), 10);

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, false);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 10);
    }

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, true);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 7);
    }

    for (int i = 0; i < 13; ++i) {
        lattice.addLooseVertex();
    }
    BOOST_CHECK_EQUAL(lattice.countAllVertices(), 23);

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, false);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 23);
    }

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, true);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 20);
    }

    lattice.appendStringWithSymbols("ąć");
    BOOST_CHECK_EQUAL(lattice.countAllVertices(), 25);

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, false);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 25);
    }

    {
        int count = 0;
        Lattice::VertexIterator vi(lattice, true);
        while (vi.hasNext()) {
            vi.next();
            ++count;
        }
        BOOST_CHECK_EQUAL(count, 22);
    }

}


BOOST_AUTO_TEST_CASE( planes ) {
    AnnotationItemManager aim;
    Lattice lattice(aim, "abcd");

    LayerTagCollection tagsFoo(lattice.getLayerTagManager().createSingletonTagCollection("foo"));
    LayerTagCollection tagsBar(lattice.getLayerTagManager().createSingletonTagCollection("bar"));
    LayerTagCollection tagsP(lattice.getLayerTagManager().createSingletonTagCollection("!plane"));
    BOOST_CHECK(lattice.getLayerTagManager().areInTheSamePlane(tagsFoo, tagsBar));
    BOOST_CHECK(!lattice.getLayerTagManager().areInTheSamePlane(tagsFoo, tagsP));

    Lattice::VertexDescriptor from = lattice.getVertexForRawCharIndex(1);
    Lattice::VertexDescriptor to = lattice.getVertexForRawCharIndex(3);
    AnnotationItem item("item");

    int edgeCount = 0;
    Lattice::InOutEdgesIterator ei = lattice.outEdges(from, lattice.getLayerTagManager().anyTag());
    while (ei.hasNext()) {
        if (lattice.getEdgeTarget(ei.next()) == to) {
            ++edgeCount;
        }
    }
    BOOST_CHECK_EQUAL(edgeCount, 0);

    lattice.addEdge(from, to, item, tagsFoo);

    edgeCount = 0;
    ei = lattice.outEdges(from, lattice.getLayerTagManager().anyTag());
    while (ei.hasNext()) {
        if (lattice.getEdgeTarget(ei.next()) == to) {
            ++edgeCount;
        }
    }
    BOOST_CHECK_EQUAL(edgeCount, 1);

    lattice.addEdge(from, to, item, tagsBar);

    edgeCount = 0;
    ei = lattice.outEdges(from, lattice.getLayerTagManager().anyTag());
    while (ei.hasNext()) {
        if (lattice.getEdgeTarget(ei.next()) == to) {
            ++edgeCount;
        }
    }
    BOOST_CHECK_EQUAL(edgeCount, 1);

    lattice.addEdge(from, to, item, tagsP);

    edgeCount = 0;
    ei = lattice.outEdges(from, lattice.getLayerTagManager().anyTag());
    while (ei.hasNext()) {
        if (lattice.getEdgeTarget(ei.next()) == to) {
            ++edgeCount;
        }
    }
    BOOST_CHECK_EQUAL(edgeCount, 2);

    edgeCount = 0;
    ei = lattice.outEdges(from, lattice.getLayerTagManager().getMask("!plane"));
    while (ei.hasNext()) {
        if (lattice.getEdgeTarget(ei.next()) == to) {
            ++edgeCount;
        }
    }
    BOOST_CHECK_EQUAL(edgeCount, 1);
}


BOOST_AUTO_TEST_SUITE_END()
