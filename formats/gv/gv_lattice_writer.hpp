#ifndef GV_LATTICE_WRITER_HDR
#define GV_LATTICE_WRITER_HDR

#include <list>
#include <set>
#include <string>

#include <boost/foreach.hpp>

#include "lattice_writer.hpp"
#include "lattice_writer_factory.hpp"
#include "aligning_writer_worker.hpp"
#include "plugin/graphviz_adapter_interface.hpp"

#include "psi_quoter.hpp"


class GVLatticeWriter : public LatticeWriter<std::ostream> {

public:
    GVLatticeWriter(
        bool showTags,
        bool color,
        std::set<std::string> filter,
        std::string outputFormat,
        bool tree,
        bool align,
        bool showSymbolEdges,
        bool disambig
    );

    ~GVLatticeWriter();

    virtual std::string getFormatName();

    GraphvizAdapterInterface * getAdapter();
    bool isActive();

    class Factory : public LatticeWriterFactory<std::ostream> {
    private:
        virtual LatticeWriter<std::ostream>* doCreateLatticeWriter(
            const boost::program_options::variables_map& options);

        virtual boost::program_options::options_description doOptionsHandled();

        virtual std::string doGetName() const;
        virtual boost::filesystem::path doGetFile() const;
    };

    bool isShowTags() const { return showTags_; }
    bool isColor() const { return color_; }
    bool isInFilter(std::string tag) { return filter_.empty() || filter_.count(tag); }

    bool areSomeInFilter(std::list<std::string> & tags) {
        if (filter_.empty()) return true;
        BOOST_FOREACH(std::string tag, tags) {
            if (filter_.count(tag)) return true;
        }
        return false;
    }

    std::string getOutputFormat() const { return outputFormat_; }
    bool isTree() const { return tree_; }
    bool isAlign() const { return align_; }
    bool isShowSymbolEdges() const { return showSymbolEdges_; }
    bool isDisambig() const { return disambig_; }

private:
    virtual std::string doInfo();

    class Worker : public AligningWriterWorker {
    public:
        Worker(GVLatticeWriter& processor,
               std::ostream& outputStream,
               Lattice& lattice);

        virtual void doRun();

        void printEdge(
            Lattice::EdgeDescriptor edge,
            PsiQuoter &quoter,
            int &ordinal,
            std::map<Lattice::EdgeDescriptor, int> &edgeOrdinalMap,
            std::map<int, int> &vertexNodes,
            std::set<int> &startVertices);

        virtual ~Worker();
    private:
        GVLatticeWriter& processor_;
    };

    virtual WriterWorker<std::ostream>* doCreateWriterWorker(
        std::ostream& outputStream, Lattice& lattice) {

        return new Worker(*this, outputStream, lattice);
    }

    bool showTags_;
    bool color_;
    std::set<std::string> filter_;
    std::string outputFormat_;
    bool tree_;
    bool align_;
    bool showSymbolEdges_;
    bool disambig_;

    GraphvizAdapterInterface * adapter_;

    static const std::string DISCARDED_STYLE;

};


#endif
