#include "tests.hpp"

#include <string>

#include <boost/scoped_ptr.hpp>

#include "av_ai_converter.hpp"
#include "chart.tpl"
#include "lattice.hpp"
#include "lattice_preparators.hpp"
#include "psi_lattice_reader.hpp"
#include "psi_lattice_writer.hpp"
#include "registrar.tpl"
#include "test_helpers.hpp"


#define SIMPLE_CHART(CH, LATTICE) \
    registrar<std::string> reg; \
    AV_AI_Converter av_ai_converter((LATTICE).getAnnotationItemManager(), reg, reg); \
    typedef chart<std::string, double, int, int_rule> simple_chart; \
    simple_chart (CH)((LATTICE), av_ai_converter);


class int_rule {
private:
    int rule_no_;

public:
    int_rule(int rule_no) : rule_no_(rule_no) { }

    int category() const { return rule_no_; }
    double score() const { return 0.0; }
    int rule_no() const { return rule_no_; }
    int tree_choice() const { return rule_no_; }
};


BOOST_AUTO_TEST_SUITE( gobio_chart )


BOOST_AUTO_TEST_CASE( chart_vertices ) {

    AnnotationItemManager aim;
    Lattice lattice(aim);
    SIMPLE_CHART(ch, lattice);

    BOOST_CHECK_EQUAL(count_vertices(ch), 1);

    lattice.appendString(std::string(9, 'a'));

    BOOST_CHECK_EQUAL(count_vertices(ch), 10);

    lattice.appendStringWithSymbols(std::string(15, 'a'));

    BOOST_CHECK_EQUAL(count_vertices(ch), 25);

    lattice.appendStringWithSymbols(std::string(5000, 'a'));

    BOOST_CHECK_EQUAL(count_vertices(ch), 5025);

}


BOOST_AUTO_TEST_CASE( chart_vertices_with_preparator ) {

    AnnotationItemManager aim;
    Lattice lattice(aim);
    SIMPLE_CHART(ch, lattice);

    BOOST_CHECK_EQUAL(count_vertices(ch), 1);

    lattice_preparators::prepareLatticeWithOneSymbolTokens(lattice, "abccdddd");

    BOOST_CHECK_EQUAL(count_vertices(ch), 9);

}


BOOST_AUTO_TEST_CASE( chart_edges ) {

    AnnotationItemManager aim;
    Lattice lattice(aim, std::string(7, 's'));
    SIMPLE_CHART(ch, lattice);

    BOOST_CHECK_EQUAL(count_vertices(ch), 8);

    std::vector<simple_chart::vertex_descriptor> v(8);
    for (int i=0; i<8; ++i) {
        v[i] = lattice.getVertexForRawCharIndex(i);
    }

    BOOST_CHECK_EQUAL(count_vertices(ch), 8);

    BOOST_CHECK_EQUAL(count_out_edges(ch), 0);
    BOOST_CHECK_EQUAL(count_in_edges(ch), 0);

    ch.add_edge(v[1], v[2], "a", 2.0, 0);
    ch.add_edge(v[2], v[3], "b", -1.0, 515);
    ch.add_edge(v[0], v[1], "a", 3.0, 1);
    ch.add_edge(v[3], v[6], "e", 1.5, 0);
    ch.add_edge(v[6], v[7], "a", 1.1, 0);

    ch.add_edge(v[6], v[7], "b", 1.1, 0);
    ch.add_edge(v[6], v[7], "c", 1.1, 0);
    ch.add_edge(v[4], v[7], "c", 2.555, 23);
    ch.add_edge(v[3], v[4], "c", 2.555, 23);
    ch.add_edge(v[0], v[4], "c", 2.555, 23);

    ch.add_edge(v[6], v[7], "d", 1.3, 1);

    BOOST_CHECK_EQUAL(count_out_edges(ch), 11);
    BOOST_CHECK_EQUAL(count_in_edges(ch), 11);

    BOOST_CHECK_EQUAL(count_marked_out_edges(ch), 0);
    BOOST_CHECK_EQUAL(count_marked_in_edges(ch), 0);

    BOOST_CHECK_EQUAL(count_vertices(ch), 8);

    BOOST_CHECK(is_consistent(ch));

}


BOOST_AUTO_TEST_CASE( chart_mask ) {

    AnnotationItemManager aim;
    Lattice lattice(aim);
    lattice_preparators::prepareLatticeWithOneSymbolTokens(lattice, "abccdddd");
    SIMPLE_CHART(ch, lattice);

    BOOST_CHECK_EQUAL(count_out_edges(ch), 8);
    BOOST_CHECK_EQUAL(count_in_edges(ch), 8);

}


BOOST_AUTO_TEST_CASE( chart_empty ) {

    AnnotationItemManager aim;
    Lattice lattice(aim, std::string(2012, 's'));
    SIMPLE_CHART(ch, lattice);

    BOOST_CHECK_EQUAL(count_out_edges(ch), 0);
    BOOST_CHECK_EQUAL(count_in_edges(ch), 0);

    BOOST_CHECK_EQUAL(count_marked_out_edges(ch), 0);
    BOOST_CHECK_EQUAL(count_marked_in_edges(ch), 0);

    BOOST_CHECK_EQUAL(count_vertices(ch), 2013);

    BOOST_CHECK(is_consistent(ch));

}


BOOST_AUTO_TEST_CASE( chart_linear ) {

    const int nb_vertices = 200;

    AnnotationItemManager aim;
    Lattice lattice(aim, std::string(nb_vertices-1, 's'));
    SIMPLE_CHART(ch, lattice);

    for (int i=0; i<nb_vertices; ++i) {
        if (i > 0) {
            for (char c='A'; c<='Z'; ++c) {
                ch.add_edge(
                    lattice.getVertexForRawCharIndex(i-1),
                    lattice.getVertexForRawCharIndex(i),
                    std::string(1, c),
                    1.4,
                    1
                );
            }
        }
    }

    BOOST_CHECK_EQUAL(count_out_edges(ch), 26*(nb_vertices-1));
    BOOST_CHECK_EQUAL(count_in_edges(ch), 26*(nb_vertices-1));

    BOOST_CHECK_EQUAL(count_marked_out_edges(ch), 0);
    BOOST_CHECK_EQUAL(count_marked_in_edges(ch), 0);

    BOOST_CHECK_EQUAL(count_vertices(ch), nb_vertices);

    BOOST_CHECK(is_consistent(ch));

}


BOOST_AUTO_TEST_CASE( chart_complete ) {

    const int nb_vertices = 30;

    AnnotationItemManager aim;
    Lattice lattice(aim, std::string(nb_vertices-1, 's'));
    SIMPLE_CHART(ch, lattice);

    for (int i=0; i<nb_vertices; ++i) {
        for (int j=0; j<i; ++j) {
            for (char c='A'; c<='Z'; ++c) {
                ch.add_edge(
                    lattice.getVertexForRawCharIndex(j),
                    lattice.getVertexForRawCharIndex(i),
                    std::string(1, c),
                    1.3,
                    1
                );
            }
        }
    }

    BOOST_CHECK_EQUAL(count_out_edges(ch), 26*(nb_vertices*(nb_vertices-1))/2);
    BOOST_CHECK_EQUAL(count_in_edges(ch), 26*(nb_vertices*(nb_vertices-1))/2);

    BOOST_CHECK_EQUAL(count_marked_out_edges(ch), 0);
    BOOST_CHECK_EQUAL(count_marked_in_edges(ch), 0);

    BOOST_CHECK_EQUAL(count_vertices(ch), nb_vertices);

    BOOST_CHECK(is_consistent(ch));

}


BOOST_AUTO_TEST_CASE( chart_marked ) {

    const int nb_vertices = 60;

    AnnotationItemManager aim;
    Lattice lattice(aim, std::string(nb_vertices-1, 's'));
    SIMPLE_CHART(ch, lattice);

    std::vector<simple_chart::edge_descriptor> e;

    for (int i=0; i<nb_vertices; ++i) {
        if (i > 0) {

            e.push_back(ch.add_edge(
                lattice.getVertexForRawCharIndex(i-1),
                lattice.getVertexForRawCharIndex(i),
                "a",
                1.4,
                1
            ).first);

            ch.mark_edge(ch.add_edge(
                lattice.getVertexForRawCharIndex(i-1),
                lattice.getVertexForRawCharIndex(i),
                "b",
                1.4,
                1
            ).first);

            ch.add_edge(
                lattice.getVertexForRawCharIndex(i-1),
                lattice.getVertexForRawCharIndex(i),
                "e",
                1.4,
                1
            );

        }
    }

    for (int i=nb_vertices/3; i<(nb_vertices/3)*2; ++i) {
        for (int j=nb_vertices/3; j<i; ++j) {

            ch.add_edge(
                lattice.getVertexForRawCharIndex(j),
                lattice.getVertexForRawCharIndex(i),
                "c",
                1.4,
                1
            );

            ch.mark_edge(ch.add_edge(
                lattice.getVertexForRawCharIndex(j),
                lattice.getVertexForRawCharIndex(i),
                "d",
                1.4,
                1
            ).first);

        }
    }

    for (int i=0; i<nb_vertices-1; ++i) {
        ch.mark_edge(e[i]);
    }

    int expected_nb_edges = (nb_vertices-1)*3 + (nb_vertices/3)*(nb_vertices/3-1);

    BOOST_CHECK_EQUAL(count_out_edges(ch), expected_nb_edges);
    BOOST_CHECK_EQUAL(count_in_edges(ch), expected_nb_edges);

    int expected_nb_out_edges = (nb_vertices-1)*2 + ((nb_vertices/3)*(nb_vertices/3-1))/2;

    BOOST_CHECK_EQUAL(count_marked_out_edges(ch), expected_nb_out_edges);
    BOOST_CHECK_EQUAL(count_marked_in_edges(ch), expected_nb_out_edges);

    BOOST_CHECK_EQUAL(count_vertices(ch), nb_vertices);

    BOOST_CHECK(is_consistent(ch));

}


BOOST_AUTO_TEST_CASE( chart_partitions ) {

    const int nb_vertices = 11;

    AnnotationItemManager aim;
    Lattice lattice(aim, std::string(nb_vertices-1, 's'));
    SIMPLE_CHART(ch, lattice);

    std::vector<simple_chart::edge_descriptor> e;

    for (int i=0; i<nb_vertices; ++i) {
        if (i > 0) {

            simple_chart::edge_descriptor edge_b = ch.add_edge(
                lattice.getVertexForRawCharIndex(i-1),
                lattice.getVertexForRawCharIndex(i),
                "b",
                1.4,
                1
            ).first;

            simple_chart::edge_descriptor edge_a = ch.add_edge(
                lattice.getVertexForRawCharIndex(i-1),
                lattice.getVertexForRawCharIndex(i),
                "a",
                1.4,
                1
            ).first;

            ch.add_edge(
                lattice.getVertexForRawCharIndex(i-1),
                lattice.getVertexForRawCharIndex(i),
                "a",
                1.5,
                2,
                edge_b
            );

            ch.add_partition(edge_a, 1.6, 3, edge_b);

            e.push_back(edge_b);
        }
    }

    for (int i=0; i<nb_vertices-1; i+=2) {

        simple_chart::edge_descriptor edge = ch.add_edge(
            lattice.getVertexForRawCharIndex(i),
            lattice.getVertexForRawCharIndex(i+2),
            "c",
            1.4,
            10
        ).first;

        ch.add_edge(
            lattice.getVertexForRawCharIndex(i),
            lattice.getVertexForRawCharIndex(i+2),
            "d",
            1.4,
            1,
            edge
        );

        ch.add_edge(
            lattice.getVertexForRawCharIndex(i),
            lattice.getVertexForRawCharIndex(i+2),
            "c",
            1.5,
            2,
            e[i/2],
            e[i/2+1]
        );

        ch.add_partition(edge, 1.6, 3, e[i/2], e[i/2+1]);

    }

    int expected_nb_edges = 2*(nb_vertices-1) + 2 *((nb_vertices-1)/2);

    BOOST_CHECK_EQUAL(count_out_edges(ch), expected_nb_edges);
    BOOST_CHECK_EQUAL(count_in_edges(ch), expected_nb_edges);

    BOOST_CHECK_EQUAL(count_marked_out_edges(ch), 0);
    BOOST_CHECK_EQUAL(count_marked_in_edges(ch), 0);

    BOOST_CHECK_EQUAL(count_vertices(ch), nb_vertices);

    BOOST_CHECK(is_consistent(ch));

    boost::scoped_ptr<LatticeWriter<std::ostream> > writer(new PsiLatticeWriter(
        false // with header
    ));
    std::ostringstream osstr;
    writer->writeLattice(lattice, osstr);
    std::string line;
    std::string contents;
    std::ifstream s(ROOT_DIR "tools/parsers/gobio/t/files/chart_partitions.psi");
    while (getline(s, line)) {
        contents += line;
        contents += "\n";
    }
    BOOST_CHECK_EQUAL(osstr.str(), contents);

}


BOOST_AUTO_TEST_CASE( chart_categories ) {

    AnnotationItemManager aim;
    Lattice lattice(aim, "");
    boost::scoped_ptr<StreamLatticeReader> reader(new PsiLatticeReader());
    reader->readIntoLattice(ROOT_DIR "tools/parsers/gobio/t/files/categories.psi", lattice);

    SIMPLE_CHART(ch, lattice);

    BOOST_CHECK_EQUAL(count_out_edges(ch), 10);
    BOOST_CHECK_EQUAL(count_in_edges(ch), 10);
    BOOST_CHECK_EQUAL(count_vertices(ch), 13);

    BOOST_CHECK_EQUAL(
        ch.edge_category(
            lattice.firstOutEdge(lattice.getFirstVertex(),
            lattice.getLayerTagManager().getMask("normalization"))),
        "'niebieskiego'");
    BOOST_CHECK_EQUAL(
        ch.edge_category(
            lattice.firstOutEdge(lattice.getFirstVertex(),
            lattice.getLayerTagManager().getMask("term"))),
        "'niebieskiego'");
    BOOST_CHECK_EQUAL(
        ch.edge_category(
            lattice.firstOutEdge(lattice.getFirstVertex(),
            lattice.getLayerTagManager().getMask("token"))),
        "'niebieskiego'");
    BOOST_CHECK_EQUAL(
        ch.edge_category(
            lattice.firstOutEdge(lattice.getFirstVertex(),
            lattice.getLayerTagManager().getMask("form"))),
        "'$niebieski'");

}


BOOST_AUTO_TEST_SUITE_END()
