#ifndef LATTICE_HDR
#define LATTICE_HDR


#include <string>
#include <list>
#include <map>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/foreach.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

#include "utf8/utf8.h"

#include "annotation_item.hpp"
#include "annotation_item_manager.hpp"
#include "exceptions.hpp"
#include "hash_wrapper.hpp"
#include "layer_tag_manager.hpp"

#include "cutter.hpp"

#include "zvalue.hpp"

/*!
  Lattice is used to keep all the information extracted by annotators
  (processors). Language units (tokens, words, phrases etc.)
  recognised by annotators are represented by edges (whereas vertices
  represent points between characters). Each edge is assigned an annotation
  item which is composed of a category and a attribute-value matrix (AVM).
  Each edge has tags which can be used to distinguish various levels, annotators etc.

  Lattice lifecycle:
  - construction, edge is created for each character (optimised!) [tag: raw]
  - formatter, extra edges for mark-ups and entity-likes (one edge "<"
    spanning "&lt;"), [text, mark-ups]
  - splitter - working on text level, Translatica splitter with
    function, get_text (....) [sentence]
  - tokeniser - working on text level (inside sentence?), get_edge_text [token]
  - lemmatiser - working on token level [lemma]
  - corrector - working on text level (if token without lemma),
  - parser - working ok token/lemma level, taking into account blanks
  - chooser - working on parser level
*/


class Lattice {

public:

    struct VertexEntry;
    struct EdgeEntry;

    typedef boost::adjacency_list<
        boost::vecS,
        boost::vecS,
        boost::bidirectionalS,
        VertexEntry,
        EdgeEntry
    > Graph;

    struct EdgeDescriptor;


    struct EdgeDescriptorWrapperToFoolBoost146OrGnu461;

    struct VertexEntry {
        /**
         * Vertex's position in text.
         * For loose vertices: index < 0.
         */
        int index;

        std::vector< std::list<EdgeDescriptorWrapperToFoolBoost146OrGnu461> > outEdgesIndex;
        std::vector< std::list<EdgeDescriptorWrapperToFoolBoost146OrGnu461> > inEdgesIndex;

        VertexEntry() : index(-1) { }
        VertexEntry(int ix) : index(ix) { }
    };

    typedef double Score;

    struct Partition;

    struct EdgeEntry {
        AnnotationItem item;
        LayerTagCollection tagList;
        Score score;
        std::list<Partition> partitions;

        EdgeEntry():
            item(AnnotationItem("")),
            tagList(LayerTagManager().createSingletonTagCollection("")),
            score(0.0)
        { }

        EdgeEntry(
            AnnotationItem aItem,
            LayerTagCollection aTagList,
            Score aScore,
            Partition aPartition
        ): item(aItem), tagList(aTagList), score(aScore) {
            partitions.push_back(aPartition);
        }
    };

    class EdgeUsage;

    class EdgeSequence {
    public:
        class Iterator {
        public:
            Iterator(Lattice & lattice, const EdgeSequence & edgeSequence);
            bool hasNext();
            EdgeDescriptor next();
            EdgeUsage nextUsage();
        private:
            Lattice & lattice_;
            const EdgeSequence & edgeSequence_;
            std::vector<EdgeUsage>::const_iterator ei_;
            int si_;
        };

        EdgeSequence();

        EdgeDescriptor firstEdge(Lattice & lattice) const;

        EdgeDescriptor lastEdge(Lattice & lattice) const;

        EdgeDescriptor nthEdge(Lattice & lattice, size_t index) const;
        zvalue nthRole(Lattice & lattice, size_t index) const;

        LayerTagCollection gatherPlaneTags(Lattice& lattice) const;

        bool empty() const;

        size_t size(Lattice & lattice) const;

        class Builder {
        public:
            Builder(Lattice & lattice) : lattice_(lattice), begin(0), end(0) { }
            Builder& addEdge(EdgeDescriptor edge, zvalue role = NULL_ZVALUE);
            EdgeSequence build();
        private:
            Lattice & lattice_;
            std::vector<EdgeUsage> links;
            int begin;
            int end;
        };

        friend class Iterator;

    private:
        std::vector<EdgeUsage> links;
        int begin;
        int end;

        EdgeSequence(const std::vector<EdgeDescriptor>& aLinks);
        EdgeSequence(const std::vector<EdgeUsage>& aLinks);
        EdgeSequence(int aBegin, int aEnd);
    };

    class Partition {

    public:
        class Iterator {
        public:
            Iterator(Lattice & lattice, const Partition & partition)
                : iter_(lattice, partition.getSequence()) { }
            bool hasNext() { return iter_.hasNext(); }
            EdgeDescriptor next() { return iter_.next(); }
            EdgeUsage nextUsage() { return iter_.nextUsage(); }
        private:
            EdgeSequence::Iterator iter_;
        };

        Partition(LayerTagCollection aTagList,
                  EdgeSequence aSequence = EdgeSequence(),
                  Score aScore = 0,
                  int aRuleId = -1,
                  int aTreeChoice = -1);

        const EdgeSequence& getSequence() const;

        const LayerTagCollection& getTagList() const;

        const Score& getScore() const;

        const int& getRuleId() const;
        const int& getTreeChoice() const;

        EdgeDescriptor firstEdge(Lattice & lattice) const { return sequence_.firstEdge(lattice); }

        EdgeDescriptor lastEdge(Lattice & lattice) const { return sequence_.lastEdge(lattice); }

    private:
        EdgeSequence sequence_;
        LayerTagCollection tagList_;
        Score score_;
        int ruleId_;
        int treeChoice_;

    };

    struct EdgeDescriptor {
        Graph::edge_descriptor descriptor;

        /**
         * Indicates the position of source vertex if the edge is an implicit edge.
         * If the edge is an explicit edge, its implicit index is negative.
         */
        int implicitIndex;

        EdgeDescriptor() : descriptor(), implicitIndex(-1) { }
        EdgeDescriptor(int implicitIx) : descriptor(), implicitIndex(implicitIx) { }
        EdgeDescriptor(const Graph::edge_descriptor& ed) : descriptor(ed), implicitIndex(-1) { }

        bool operator<(EdgeDescriptor other) const {
            if (implicitIndex < 0 && other.implicitIndex < 0) {
                return descriptor < other.descriptor;
            }
            return implicitIndex < other.implicitIndex;
        }

        bool operator==(EdgeDescriptor other) const {
            if (implicitIndex < 0 && other.implicitIndex < 0) {
                return descriptor == other.descriptor;
            }
            return implicitIndex == other.implicitIndex;
        }

        bool isExplicit() const {
            return implicitIndex < 0;
        }
    };

    struct EdgeDescriptorWrapperToFoolBoost146OrGnu461 : public Graph::edge_descriptor {
    public:
        EdgeDescriptorWrapperToFoolBoost146OrGnu461() {}
        EdgeDescriptorWrapperToFoolBoost146OrGnu461(const Graph::edge_descriptor& ed)
            :Graph::edge_descriptor(ed) {}
    };

    /**
     * Edge used as a contituent in some superconstituent, i.e.
     * an edge with possibly some "role" (usually null,
     * in a parser ouput can express some syntactic role, e.g.
     * being a subject, an object etc.).
     */
    class EdgeUsage {
    public:
        explicit EdgeUsage(EdgeDescriptor edge);
        EdgeUsage(EdgeDescriptor edge, zvalue role);

        EdgeDescriptor getEdge() const;
        zvalue getRole() const;
    private:
        EdgeDescriptor edge_;
        zvalue role_;
    };

    /**
     * Vertex descriptor is a number.
     * For non-loose vertices, it is a non-negative integer indicating position in the text
     * (counted in bytes).
     * For loose vertices, vertex descriptor is an arbitrary negative integer.
     */
    typedef int VertexDescriptor;

    typedef Graph::edge_iterator EdgeIterator;
    typedef Graph::out_edge_iterator OutEdgeIterator;
    typedef Graph::in_edge_iterator InEdgeIterator;

    class VertexIterator {
    public:
        VertexIterator(Lattice& lattice, bool skipUselessVertices = true);
        bool hasNext();
        VertexDescriptor next();
    private:
        Lattice& lattice_;
        VertexDescriptor vd_;
        bool withLooseVertices_;
        bool skipUselessVertices_;
        std::vector< std::pair<Lattice::VertexDescriptor, int> > iterContainer_;
        std::vector< std::pair<Lattice::VertexDescriptor, int> >::iterator ici_;

        void nextRealVertex_();
        bool shouldBeSkipped_(Lattice::VertexDescriptor vd);

        int f_(Graph::vertex_descriptor vertex);
    };

    class InOutEdgesIterator {
    public:
        InOutEdgesIterator(
            std::list<EdgeDescriptorWrapperToFoolBoost146OrGnu461>::const_iterator begin,
            std::list<EdgeDescriptorWrapperToFoolBoost146OrGnu461>::const_iterator end,
            int implicitIndex = -1
        ) :
            type_(EDGE_DESCRIPTOR_ITER),
            edi_(begin),
            ediEnd_(end),
            implicitIndex_(implicitIndex)
        { }

        InOutEdgesIterator(
            std::pair<OutEdgeIterator, OutEdgeIterator> ir,
            int implicitIndex = -1
        ) :
            type_(OUT_EDGE_ITER),
            oei_(ir.first),
            oeiEnd_(ir.second),
            implicitIndex_(implicitIndex)
        { }

        InOutEdgesIterator(
            std::pair<InEdgeIterator, InEdgeIterator> ir,
            int implicitIndex = -1
        ) :
            type_(IN_EDGE_ITER),
            iei_(ir.first),
            ieiEnd_(ir.second),
            implicitIndex_(implicitIndex)
        { }

        InOutEdgesIterator(
            int implicitIndex = -1
        ) :
            type_(IMPLICIT_ITER),
            implicitIndex_(implicitIndex)
        { }

        bool hasNext();
        EdgeDescriptor next();
    private:
        enum {EDGE_DESCRIPTOR_ITER, OUT_EDGE_ITER, IN_EDGE_ITER, IMPLICIT_ITER} type_;
        std::list<EdgeDescriptorWrapperToFoolBoost146OrGnu461>::const_iterator edi_;
        std::list<EdgeDescriptorWrapperToFoolBoost146OrGnu461>::const_iterator ediEnd_;
        OutEdgeIterator oei_;
        OutEdgeIterator oeiEnd_;
        InEdgeIterator iei_;
        InEdgeIterator ieiEnd_;
        int implicitIndex_;
    };

    class SortedEdgesIterator {
    public:
        SortedEdgesIterator(Lattice& lattice, LayerTagMask mask);
        bool hasNext();
        EdgeDescriptor next();
        virtual ~SortedEdgesIterator();
    protected:
        Lattice& lattice_;
        LayerTagMask mask_;
        InOutEdgesIterator ei_;
    private:
        VertexIterator vi_;
        virtual InOutEdgesIterator getEdgesIterator_(VertexDescriptor vd) = 0;
    };

    class EdgesSortedBySourceIterator : public SortedEdgesIterator {
    public:
        EdgesSortedBySourceIterator(Lattice& lattice, LayerTagMask mask);
    private:
        virtual InOutEdgesIterator getEdgesIterator_(VertexDescriptor vd);
    };

    class EdgesSortedByTargetIterator : public SortedEdgesIterator {
    public:
        EdgesSortedByTargetIterator(Lattice& lattice, LayerTagMask mask);
    private:
        virtual InOutEdgesIterator getEdgesIterator_(VertexDescriptor vd);
    };

    /**
     * Creates an empty lattice.
     */
    Lattice(AnnotationItemManager & annotationItemManager);

    /**
     * Creates a lattice with text `text` and no edges.
     */
    Lattice(AnnotationItemManager & annotationItemManager, const std::string & text);

    ~Lattice();

    /**
     * Appends string `text` at the end of the lattice (without adding any edges).
     */
    void appendString(const std::string & text);

    /**
     * For each character between `startVertex` and `endVertex`
     * an edge labelled with layer tag `symbol`
     * and category `'c` (where `c` is a given character) is added.
     */
    void addSymbols(VertexDescriptor startVertex, VertexDescriptor endVertex);

    /**
     * Appends string `text` at the end of the lattice and adds symbols to this string.
     */
    void appendStringWithSymbols(const std::string & text);

    /**
     * Adds new loose vertex to the lattice.
     * Returns vertex descriptor of added vertex.
     */
    VertexDescriptor addLooseVertex();

    /**
     * Returns the vertex for `ix`-th character of text.
     * If index `ix` is out of range, a NoVertexException is thrown.
     */
    VertexDescriptor getVertexForRawCharIndex(int ix) const;

    /**
     * Returns the first vertex (the same as getVertexForRawCharIndex(0)).
     */
    VertexDescriptor getFirstVertex() const;

    /**
     * Returns the last vertex.
     */
    VertexDescriptor getLastVertex();

    /**
     * Returns vertex's raw char index.
     * If vertex is loose, a WrongVertexException is thrown.
     */
    size_t getVertexRawCharIndex(VertexDescriptor vd);

    /**
     * Adds an edge from vertex `from` to vertex `to` with `annotationItem`
     * and `tags` as layer tags. The partition of the edge into subedges
     * is given with `sequence` argument.
     *
     * A new will not be added if there is already an edge from vertex `from`
     * to vertex `to` of annotation `annotation_item` (tags and partitions will
     * be updated in such a case).
     */
    EdgeDescriptor addEdge(VertexDescriptor from,
                           VertexDescriptor to,
                           const AnnotationItem& annotationItem,
                           LayerTagCollection tags,
                           EdgeSequence sequence = EdgeSequence(),
                           Score score = 0.0,
                           int ruleId = -1,
                           int treeChoice = -1);

    EdgeDescriptor addPartitionToEdge(EdgeDescriptor edge,
                                      LayerTagCollection tags,
                                      EdgeSequence sequence = EdgeSequence(),
                                      Score score = 0.0,
                                      int ruleId = -1,
                                      int treeChoice = -1);

    /**
     * Discard the given edge.
     *
     * The edge won't be removed - will be labeled with `discarded` tag.
     */
    void discard(EdgeDescriptor edge);

    /**
     * Returns outgoing edges which has at least one layer tag from `mask`.
     */
    InOutEdgesIterator outEdges(
        VertexDescriptor vertex,
        LayerTagMask mask
    );

    /**
     * Returns ingoing edges which has at least one layer tag from `mask`.
     */
    InOutEdgesIterator inEdges(
        VertexDescriptor vertex,
        LayerTagMask mask
    );

    InOutEdgesIterator allOutEdges(VertexDescriptor vertex);
    InOutEdgesIterator allInEdges(VertexDescriptor vertex);

    EdgeDescriptor firstOutEdge(VertexDescriptor vertex, LayerTagMask mask);
    EdgeDescriptor firstInEdge(VertexDescriptor vertex, LayerTagMask mask);

    /**
     * returns the list of edges which have at least one layer tag from `mask`
     * sorted by source vertex
     */
    EdgesSortedBySourceIterator edgesSortedBySource(LayerTagMask mask);

    EdgesSortedBySourceIterator allEdgesSortedBySource();

    /**
     * returns the list of edges which have at least one layer tag from `mask`
     * sorted by target vertex
     */
    EdgesSortedByTargetIterator edgesSortedByTarget(LayerTagMask mask);

    EdgesSortedByTargetIterator allEdgesSortedByTarget();

    LayerTagManager& getLayerTagManager();
    AnnotationItemManager& getAnnotationItemManager();

    const AnnotationItem getEdgeAnnotationItem(EdgeDescriptor edge);
    const LayerTagCollection& getEdgeLayerTags(EdgeDescriptor edge) const;

    /**
     * Returns edge source vertex.
     */
    VertexDescriptor getEdgeSource(EdgeDescriptor edge) const;

    /**
     * Returns edge target vertex.
     */
    VertexDescriptor getEdgeTarget(EdgeDescriptor edge) const;

    /**
     * Returns edge source's index if edge source is not loose.
     * Otherwise, a WrongVertexException is thrown.
     */
    int getEdgeBeginIndex(EdgeDescriptor edge) const;

    /**
     * Returns edge target's index if edge target is not loose.
     * Otherwise, a WrongVertexException is thrown.
     */
    int getEdgeEndIndex(EdgeDescriptor edge) const;

    /**
     * Returns the length of the edge linking non-loose vertices.
     * If edge's source or target is loose, a WrongVertexException is thrown.
     */
    int getEdgeLength(EdgeDescriptor edge) const;

    /**
     * Symbol edges dominated by other edges are not printed by some writers by default.
     * This method checks if the edge should not be printed.
     */
    bool isEdgeHidden(EdgeDescriptor edge) const;

    const std::list<Partition> & getEdgePartitions(EdgeDescriptor edge) const;
    Score getEdgeScore(EdgeDescriptor edge) const;

    const std::string& getAllText() const;
    const std::string getEdgeText(EdgeDescriptor edge) const;
    const StringFrag getEdgeTextAsStringFrag(EdgeDescriptor edge) const;
    std::string getSequenceText(const EdgeSequence& sequence);
    StringFrag getSequenceTextAsStringFrag(const EdgeSequence& sequence);
    std::string getPartitionText(const Partition& partition);

    const std::string getAnnotationText(EdgeDescriptor edge);
    const StringFrag getAnnotationTextAsStringFrag(EdgeDescriptor edge);
    const std::string getAnnotationCategory(EdgeDescriptor edge);

    void runCutter(Cutter& cutter, LayerTagMask mask, LayerTagMask superMask);

    bool isBlank(Lattice::EdgeDescriptor edge);

    /**
     * Get a path starting with `vertex` composed of edges matching `mask`.
     * The final vertex of the returned path will be assigned to `vertex`.
     * The path is returned as an edge sequence.
     *
     * If there are multiple outgoing edges matching `mask` the best
     * one is chosen (the last one with the higher score).
     */
    Lattice::EdgeSequence getPath(VertexDescriptor& vertex, LayerTagMask mask);

    /**
     * Get a path starting with `vertex` composed of edges matching `mask`,
     * skipping edges satisfying the given predicate.
     *
     * The final vertex of the returned path will be assigned to `vertex`.
     * The path is returned as an edge sequence.
     *
     * If there are multiple outgoing edges matching `mask` the best
     * one is chosen (the last one with the higher score).
     */
    template <typename EdgePredicate>
    Lattice::EdgeSequence getPathWithSkipping(
        VertexDescriptor& vertex,
        LayerTagMask mask,
        EdgePredicate skippingPredicate
    );

    /**
     * Get a path starting with `vertex` composed of edges matching `mask`,
     * skipping blank edges.
     *
     * The final vertex of the returned path will be assigned to `vertex`.
     * The path is returned as an edge sequence.
     *
     * If there are multiple outgoing edges matching `mask` the best
     * one is chosen (the last one with the higher score).
     */
    Lattice::EdgeSequence getPathSkippingBlanks(VertexDescriptor& vertex, LayerTagMask mask);

    /**
     * Checks if the vertex is loose.
     */
    bool isLooseVertex(VertexDescriptor vd) const;

    /**
     * Returns a unique positive number for each loose vertex.
     * Otherwise, a WrongVertexException is thrown.
     */
    int getLooseVertexIndex(VertexDescriptor vd) const;

    void correctionInsert(VertexDescriptor here, std::string text);
    void correctionErase(VertexDescriptor from, VertexDescriptor to);
    void correctionReplace(VertexDescriptor from, VertexDescriptor to, std::string text);

    /**
     * Returns the number of all edges with source `from` and target `to`.
     */
    int countEdges(VertexDescriptor from, VertexDescriptor to);

    /**
     * Returns the number of all vertices in the lattice.
     */
    int countAllVertices();

private:

    struct IsBlank {
        bool operator()(
            Lattice & lattice,
            EdgeDescriptor edge
        ) {
            return lattice.isBlank(edge);
        }
    };

    Graph graph_;

    LayerTagManager layerTagManager_;

    AnnotationItemManager & annotationItemManager_;

    /**
     * Stores the lattice text.
     */
    std::string allText_;

    /**
     * Stores the information about whether there is an implicit out-edge from given vertex.
     */
    boost::dynamic_bitset<> implicitOutEdges_;

    /**
     * Stores the information about whether given implicit out-edge is hidden
     * (ie. it is a standard "symbol" edge).
     * Implicit edges that are not hidden are visible
     * but can be made hidden by adding proper super-edges.
     */
    boost::dynamic_bitset<> hiddenImplicitOutEdges_;

    /**
     * Stores the information about whether given implicit out-edge is explicitly visible
     * (eg. when there are multiple "symbol" out-edges).
     * Explicitly visible implicit edge cannot be made hidden.
     */
    boost::dynamic_bitset<> visibleImplicitOutEdges_;

    typedef std::map<int, Graph::vertex_descriptor> VerticesMap;

    /**
     * A map mapping VertexDescriptors to boost's vertex descriptors.
     * If map does not contain given index, it means that given vertex is an implicit vertex
     * (or that there is no such vertex - if index is in the middle of multibyte UTF8 char).
     */
    VerticesMap vertices_;

    /**
     * The number of loose vertices.
     */
    int nLooseVertices_;

    typedef boost::bimap<LayerTagMask, int> TagMasksBimap;
    typedef TagMasksBimap::value_type TagMasksBimapItem;
    typedef TagMasksBimap::left_map::const_iterator TagMasksBimapLeftIterator;
    TagMasksBimap indexedTagMasks_;

    int addTagMaskIndex_(LayerTagMask tagMask);

    void resizeImplicitEdgesStructures_();

    VertexDescriptor priorVertex_(VertexDescriptor vertex);

    size_t symbolLength_(int ix) const;
    const LayerTagCollection& getSymbolTag_() const;

    int getEdgeSourceInternalIndex_(EdgeDescriptor edge) const;
    int getEdgeTargetInternalIndex_(EdgeDescriptor edge) const;

    VertexDescriptor firstSequenceVertex_(const EdgeSequence& sequence);
    VertexDescriptor lastSequenceVertex_(const EdgeSequence& sequence);

    void runCutterOnEdge_(Cutter& cutter, EdgeDescriptor edge, LayerTagMask mask);

    EdgeSequence cutSequenceByTextLength_(
        EdgeSequence::Iterator& sequenceIterator,
        int length
    );

    struct HashFun {
        HASH_WRAPPER_EXTRA_STUFF

        unsigned int operator()(
            const std::pair<
                std::pair<VertexDescriptor, VertexDescriptor>,
                AnnotationItem
            >& k
        ) const {
#ifdef __VS__
            return HASH_WRAPPER_FULL_HASH_TRAITS<int>().operator()(int(k.first.first))
                ^ HASH_WRAPPER_FULL_HASH_TRAITS<int>().operator()(int(k.first.second))
                ^ HASH_WRAPPER_FULL_HASH_TRAITS<long>().operator()(
                    k.second.getHash());
#else
            return (int(k.first.first) << 8)
                ^ int(k.first.second)
                ^ (int(k.second.getHash()) << 16);
#endif
        }

#ifdef __VS__
        bool operator()(
            const std::pair<
                std::pair<VertexDescriptor, VertexDescriptor>,
                AnnotationItem
            >& a,
            const std::pair<
                std::pair<VertexDescriptor, VertexDescriptor>,
                AnnotationItem
            >& b
        ) const {
            return a != b;
        }
#endif
    };


    struct VertexPairHashFun {
        HASH_WRAPPER_EXTRA_STUFF

        unsigned int operator()(
            const std::pair<VertexDescriptor, VertexDescriptor>& k
        ) const {
#ifdef __VS__
            return HASH_WRAPPER_FULL_HASH_TRAITS<int>().operator()(int(k.first))
                ^ HASH_WRAPPER_FULL_HASH_TRAITS<int>().operator()(int(k.second));
#else
            return (int(k.first) << 8) ^ int(k.second);
#endif
        }

#ifdef __VS__
        bool operator()(
            const std::pair<VertexDescriptor, VertexDescriptor>& a,
            const std::pair<VertexDescriptor, VertexDescriptor>& b
        ) const {
            return a != b;
        }
#endif
    };


    typedef HashWrapper3<
        std::pair<
            std::pair<VertexDescriptor, VertexDescriptor>,
            AnnotationItem
        >,
        std::vector<EdgeDescriptor>,
        HashFun
    >::type VVCHash;

    typedef HashWrapper3<
        std::pair<VertexDescriptor, VertexDescriptor>,
        int,
        VertexPairHashFun
    >::type EdgeCounterHash;

    VVCHash vvcHash_;
    EdgeCounterHash edgeCounterHash_;

    LayerTagCollection symbolTag_;
    LayerTagCollection discardedTag_;

    std::list<Partition> emptyPartitionList_;
};


template <typename EdgePredicate>
Lattice::EdgeSequence Lattice::getPathWithSkipping(
    VertexDescriptor& vertex,
    LayerTagMask mask,
    EdgePredicate skippingPredicate
) {
    bool nextVertexFound = true;

    Lattice::EdgeSequence::Builder pathBuilder(*this);

    do {
        InOutEdgesIterator iter = outEdges(vertex, mask);

        if (iter.hasNext()) {
            EdgeDescriptor bestOne = iter.next();
            while (iter.hasNext()) {
                bestOne = iter.next();
            }
            pathBuilder.addEdge(bestOne);
            vertex = getEdgeTarget(bestOne);
        } else {
            iter = allOutEdges(vertex);
            if (iter.hasNext()) {
                EdgeDescriptor bestOne = iter.next();
                while (iter.hasNext()) {
                    EdgeDescriptor currentOne = iter.next();
                    if (skippingPredicate(*this, currentOne)) {
                        bestOne = iter.next();
                    }
                }
                if (skippingPredicate(*this, bestOne)) {
                    pathBuilder.addEdge(bestOne);
                    vertex = getEdgeTarget(bestOne);
                } else {
                    nextVertexFound = false;
                }
            } else {
                nextVertexFound = false;
            }
        }

    } while (nextVertexFound);

    return pathBuilder.build();
}


#endif
