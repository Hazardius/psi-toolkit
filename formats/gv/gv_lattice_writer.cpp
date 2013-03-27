#include "gv_lattice_writer.hpp"

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <vector>

#include "lattice.hpp"
#include "plugin_manager.hpp"
#include "psi_quoter.hpp"


GVLatticeWriter::GVLatticeWriter(
    bool showTags,
    bool color,
    std::set<std::string> filter,
    std::string outputFormat,
    bool tree,
    bool align,
    bool showSymbolEdges
) :
    showTags_(showTags),
    color_(color),
    filter_(filter),
    outputFormat_(outputFormat),
    tree_(tree),
    align_(align),
    showSymbolEdges_(showSymbolEdges)
{
    adapter_ = dynamic_cast<GraphvizAdapterInterface*>(
        PluginManager::getInstance().createPluginAdapter("graphviz")
    );
}

GVLatticeWriter::~GVLatticeWriter() {
    if (adapter_) {
        PluginManager::getInstance().destroyPluginAdapter("graphviz", adapter_);
    }
}

std::string GVLatticeWriter::getFormatName() {
    return "GraphViz";
}

std::string GVLatticeWriter::doInfo() {
    return "GraphViz writer";
}

GraphvizAdapterInterface * GVLatticeWriter::getAdapter() {
    return adapter_;
}

bool GVLatticeWriter::isActive() {
    if (adapter_) {
        return true;
    } else {
        return false;
    }
}


LatticeWriter<std::ostream>* GVLatticeWriter::Factory::doCreateLatticeWriter(
    const boost::program_options::variables_map& options) {

    std::set<std::string> filter;
    if (options.count("filter")) {
        std::vector<std::string> filterVector = options["filter"].as< std::vector<std::string> >();
        filter.insert(filterVector.begin(), filterVector.end());
    }

    return new GVLatticeWriter(
        options.count("show-tags"),
        !options.count("no-color"),
        filter,
        options["format"].as<std::string>(),
        options.count("tree"),
        !options.count("no-align"),
        options.count("show-symbol-edges")
    );
}

boost::program_options::options_description GVLatticeWriter::Factory::doOptionsHandled() {
    boost::program_options::options_description optionsDescription("Allowed options");

    optionsDescription.add_options()
        ("filter", boost::program_options::value< std::vector<std::string> >()->multitoken(),
            "show only edges tagged with specified tags")
        ("format", boost::program_options::value<std::string>()->default_value("svg"),
            "choose output format")
        ("no-align",
            "allow nodes to be not aligned left to right")
        ("no-color",
            "make output monochromatic (black on white)")
        ("show-tags",
            "print edges' layer tags")
        ("show-symbol-edges",
            "show symbol edges")
        ("tree",
            "show dependencies between edges instead of the content of the lattice");

    return optionsDescription;
}


std::string GVLatticeWriter::Factory::doGetName() const {
    return "gv-writer";
}

boost::filesystem::path GVLatticeWriter::Factory::doGetFile() const {
    return __FILE__;
}


GVLatticeWriter::Worker::~Worker() { }

GVLatticeWriter::Worker::Worker(
    GVLatticeWriter& processor,
    std::ostream& outputStream,
    Lattice& lattice
) :
    AligningWriterWorker(outputStream, lattice),
    processor_(processor)
{ }


void GVLatticeWriter::Worker::doRun() {

    if (processor_.isActive()) {

        if (
            processor_.getOutputFormat() != "canon" &&
            processor_.getOutputFormat() != "dot" &&
            processor_.getOutputFormat() != "eps" &&
            processor_.getOutputFormat() != "fig" &&
            processor_.getOutputFormat() != "gd" &&
            processor_.getOutputFormat() != "gd:cairo" &&
            processor_.getOutputFormat() != "gd:gd" &&
            processor_.getOutputFormat() != "gd2" &&
            processor_.getOutputFormat() != "gd2:cairo" &&
            processor_.getOutputFormat() != "gd2:gd" &&
            processor_.getOutputFormat() != "gif" &&
            processor_.getOutputFormat() != "gif:cairo" &&
            processor_.getOutputFormat() != "gif:gd" &&
            processor_.getOutputFormat() != "gv" &&
            processor_.getOutputFormat() != "jpe" &&
            processor_.getOutputFormat() != "jpe:cairo" &&
            processor_.getOutputFormat() != "jpe:gd" &&
            processor_.getOutputFormat() != "jpeg" &&
            processor_.getOutputFormat() != "jpeg:cairo" &&
            processor_.getOutputFormat() != "jpeg:gd" &&
            processor_.getOutputFormat() != "jpg" &&
            processor_.getOutputFormat() != "jpg:cairo" &&
            processor_.getOutputFormat() != "jpg:gd" &&
            processor_.getOutputFormat() != "pdf" &&
            processor_.getOutputFormat() != "plain" &&
            processor_.getOutputFormat() != "plain-ext" &&
            processor_.getOutputFormat() != "png" &&
            processor_.getOutputFormat() != "png:cairo" &&
            processor_.getOutputFormat() != "png:gd" &&
            processor_.getOutputFormat() != "ps" &&
            processor_.getOutputFormat() != "ps:cairo" &&
            processor_.getOutputFormat() != "ps:ps" &&
            processor_.getOutputFormat() != "ps2" &&
            processor_.getOutputFormat() != "svg" &&
            processor_.getOutputFormat() != "svg:cairo" &&
            processor_.getOutputFormat() != "svg:svg" &&
            processor_.getOutputFormat() != "svgz" &&
            processor_.getOutputFormat() != "tk" &&
            processor_.getOutputFormat() != "vml" &&
            processor_.getOutputFormat() != "vmlz" &&
            processor_.getOutputFormat() != "wbmp" &&
            processor_.getOutputFormat() != "wbmp:cairo" &&
            processor_.getOutputFormat() != "wbmp:gd" &&
            processor_.getOutputFormat() != "xdot"
        ) {
            throw PsiException("Format \"" + processor_.getOutputFormat() +
                "\" not recognized. Use one of the following formats: " +
                "canon dot eps fig gd(:cairo,:gd) gd2(:cairo,:gd) gif(:cairo,:gd) gv " +
                "jpe(:cairo,:gd) jpeg(:cairo,:gd) jpg(:cairo,:gd) pdf plain plain-ext " +
                "png(:cairo,:gd) ps(:cairo,:ps) ps2 svg(:cairo,:svg) svgz tk vml vmlz " +
                "wbmp(:cairo,:gd) xdot");
        }

        char * tmpFile;
        tmpFile = tempnam(NULL, "gv_");

        std::string arg0("dot");
        std::string arg1("-T" + processor_.getOutputFormat());
        std::string arg2("-o");
        arg2 += tmpFile;

        processor_.getAdapter()->init(arg0, arg1, arg2);

        PsiQuoter quoter;

        std::map<int, int> vertexNodes;
        std::set<int> startVertices;

        std::map<Lattice::EdgeDescriptor, int> edgeOrdinalMap;
        int ordinal = 0;

        Lattice::EdgesSortedByTargetIterator ei
            = lattice_.edgesSortedByTarget(lattice_.getLayerTagManager().anyTag());

        if (processor_.isTree()) {
            processor_.getAdapter()->setRankDir("TB");
        } else {
            processor_.getAdapter()->setRankDir("LR");
        }

        while (ei.hasNext()) {

            Lattice::EdgeDescriptor edge = ei.next();

            std::list<std::string> tagNames
                = lattice_.getLayerTagManager().getTagNames(lattice_.getEdgeLayerTags(edge));

            if (
                tagNames.size() == 1 &&
                tagNames.front() == "symbol" &&
                !processor_.isShowSymbolEdges()
            ) continue;

            if (!processor_.areSomeInFilter(tagNames)) continue;

            Lattice::VertexDescriptor source = lattice_.getEdgeSource(edge);
            Lattice::VertexDescriptor target = lattice_.getEdgeTarget(edge);

            std::stringstream edgeIdSs;
            std::stringstream edgeLabelSs;

            const AnnotationItem& annotationItem = lattice_.getEdgeAnnotationItem(edge);
            if (lattice_.isLooseVertex(source) || lattice_.isLooseVertex(target)) {
                edgeLabelSs << quoter.escape(annotationItem.getText());
            } else {
                edgeLabelSs << quoter.escape(lattice_.getEdgeText(edge));
            }

            std::string tagStr("");
            std::stringstream colorSs;
            colorSs << std::setbase(16);

            if (processor_.isShowTags() || processor_.isColor()) {
                BOOST_FOREACH(std::string tagName, tagNames) {
                    if (!processor_.isInFilter(tagName)) continue;
                    if (!tagStr.empty()) {
                        tagStr += ",";
                        colorSs << ":";
                    }
                    tagStr += tagName;
                    if (processor_.isColor()) {
                        const std::collate<char>& coll
                            = std::use_facet<std::collate<char> >(std::locale());
                        unsigned int color
                            = coll.hash(tagName.data(), tagName.data() + tagName.length())
                                & 0xffffff;
                        if ((color & 0xe0e0e0) == 0xe0e0e0) {
                            color &= 0x7f7f7f;
                        } // darken if too bright
                        colorSs << "#" << std::setbase(16) << color;
                    }
                }
            }

            if (processor_.isShowTags()) {
                edgeLabelSs << " (" << tagStr << ")";
            }

            edgeLabelSs << " " << annotationItem.getCategory();

            int n;
            int m;
            int e;

            if (processor_.isTree()) {

                ++ordinal;
                edgeOrdinalMap[edge] = ordinal;
                edgeIdSs << ordinal;

                n = processor_.getAdapter()->addNode(edgeIdSs.str());
                processor_.getAdapter()->setNodeLabel(n, edgeLabelSs.str());

                if (processor_.isColor()) {
                    processor_.getAdapter()->setNodeColor(n, colorSs.str());
                }

                int partitionNumber = 0;
                std::list<Lattice::Partition> partitions = lattice_.getEdgePartitions(edge);
                BOOST_FOREACH(Lattice::Partition partition, partitions) {
                    std::stringstream partSs;
                    ++partitionNumber;
                    partSs << partitionNumber;
                    Lattice::Partition::Iterator ei(lattice_, partition);
                    while (ei.hasNext()) {
                        Lattice::EdgeDescriptor ed = ei.next();
                        std::map<Lattice::EdgeDescriptor, int>::iterator
                            moi = edgeOrdinalMap.find(ed);
                        if (moi != edgeOrdinalMap.end()) {
                            std::stringstream edSs;
                            edSs << moi->second;
                            m = processor_.getAdapter()->addNode(edSs.str());
                            e = processor_.getAdapter()->addEdge(n, m);
                            if (partitions.size() > 1) {
                                processor_.getAdapter()->setEdgeLabel(e, partSs.str());
                            }
                        }
                    }
                }

            } else {

                std::stringstream nSs;
                int nIx;
                if (lattice_.isLooseVertex(source)) {
                    nIx = lattice_.getLooseVertexIndex(source);
                    nSs << "L" << nIx;
                } else {
                    nIx = lattice_.getVertexRawCharIndex(source);
                    nSs << nIx;
                }
                n = processor_.getAdapter()->addNode(nSs.str());
                if (processor_.isAlign() && !lattice_.isLooseVertex(source)) {
                    vertexNodes.insert(std::pair<int, int>(nIx, n));
                    startVertices.insert(nIx);
                }

                std::stringstream mSs;
                int mIx;
                if (lattice_.isLooseVertex(target)) {
                    mIx = lattice_.getLooseVertexIndex(target);
                    mSs << "L" << mIx;
                } else {
                    mIx = lattice_.getVertexRawCharIndex(target);
                    mSs << mIx;
                }
                m = processor_.getAdapter()->addNode(mSs.str());
                if (processor_.isAlign() && !lattice_.isLooseVertex(target)) {
                    vertexNodes.insert(std::pair<int, int>(mIx, m));
                }

                e = processor_.getAdapter()->addEdge(n, m);

                processor_.getAdapter()->setEdgeLabel(e, edgeLabelSs.str());

                if (processor_.isColor()) {
                    processor_.getAdapter()->setEdgeColor(e, colorSs.str());
                }

            }

        }

        if (processor_.isAlign() && !processor_.isTree()) {
            std::map<int, int>::iterator vni = vertexNodes.begin();
            int prevKey = vni->first;
            int prevVal = vni->second;
            int invisibleEdge;
            ++vni;
            while (vni != vertexNodes.end()) {
                if (!startVertices.count(prevKey)) {
                    invisibleEdge = processor_.getAdapter()->addEdge(prevVal, vni->second);
                    processor_.getAdapter()->setEdgeStyle(invisibleEdge, "invis");
                }
                prevKey = vni->first;
                prevVal = vni->second;
                ++vni;
            }
        }

        processor_.getAdapter()->finalize();

        try {
            std::string line;
            std::string contents;
            std::ifstream s(tmpFile);
            while (getline(s, line)) {
                contents += line;
                contents += "\n";
            }
            print_(contents);
        } catch (...) { }

        std::remove(tmpFile);
        free(tmpFile);

    }

}
