#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assign.hpp>
#include <boost/program_options/parsers.hpp>

#include "guessing_reader.hpp"
#include "stream_helpers.hpp"

#include "txt_lattice_reader.hpp"
#include "apertium_lattice_reader.hpp"
#include "psi_lattice_reader.hpp"
#include "utt_lattice_reader.hpp"
#if HAVE_POPPLER
#include "pdf_lattice_reader.hpp"
#endif
#if HAVE_DJVULIBRE
#include "djvu_lattice_reader.hpp"
#endif
#if USE_DOC_READER
#include "doc_lattice_reader.hpp"
#endif

const int GuessingReader::DEFAULT_BLOCK_SIZE = 32;

std::map<std::string, GuessingReader::PointerToReader> GuessingReader::fileTypeToReaderMap_ =
    boost::assign::map_list_of
        ("txt",  PointerToReader(new TxtLatticeReader::Factory()))
        ("rtf", PointerToReader(new ApertiumLatticeReader::Factory()))
        ("html", PointerToReader(new ApertiumLatticeReader::Factory()))
        ("docx", PointerToReader(new ApertiumLatticeReader::Factory()))
        ("xlsx", PointerToReader(new ApertiumLatticeReader::Factory()))
        ("pptx", PointerToReader(new ApertiumLatticeReader::Factory()))
        //("tex", PointerToReader(new ApertiumLatticeReader::Factory()))
        ("psi", PointerToReader(new PsiLatticeReader::Factory()))
#if HAVE_POPPLER
        ("pdf", PointerToReader(new PDFLatticeReader::Factory()))
#endif
#if HAVE_DJVULIBRE
        ("djvu", PointerToReader(new DjVuLatticeReader::Factory()))
#endif
#if USE_DOC_READER
        ("doc", PointerToReader(new DocLatticeReader::Factory()))
#endif
        ("utt", PointerToReader(new UTTLatticeReader::Factory()));

std::map<std::string, std::string> GuessingReader::fileTypeToReaderOptionsMap_ =
    boost::assign::map_list_of
        ("html", "--format html")
        ("rtf", "--format rtf")
        ("docx", "--format docx")
        ("xlsx", "--format xlsx")
        ("pptx", "--format pptx");
        //("tex", "--format latex");

std::string GuessingReader::getFormatName() {
    return "Guessing";
}

std::string GuessingReader::doInfo() {
    return "Guessing reader";
}

GuessingReader::GuessingReader() : blockSize_(DEFAULT_BLOCK_SIZE) { }
GuessingReader::GuessingReader(int blockSize) : blockSize_(blockSize) { }


std::string GuessingReader::guessFileType(std::istream& input) {
    std::string data = getStartingDataWithoutTouchingStream(input, blockSize_);
    std::string filetype = fileRecognizer_.recognizeFileExtension(data);

    if (filetype == "zip") {
        DEBUG("compressed archive format recognized, checking inside...");

        filetype = fileRecognizer_.recognizeCompressedFileFormat(
            getDataWithoutTouchingIStream_(input));
    }

    return filetype;
}

LatticeReader<std::istream>* GuessingReader::getLatticeReader(std::string type) {
    boost::program_options::variables_map options;

    std::map<std::string, PointerToReader>::iterator foundReader
        = fileTypeToReaderMap_.find(type);
    std::map<std::string, std::string>::iterator foundOptions
        = fileTypeToReaderOptionsMap_.find(type);

    if (foundReader != fileTypeToReaderMap_.end()) {
        DEBUG("guessed reader for: " << type);

        if (foundOptions != fileTypeToReaderOptionsMap_.end()) {
            DEBUG("with options: " << foundOptions->second);

            std::vector<std::string> optionsArgv;
            boost::split(optionsArgv, foundOptions->second, boost::is_any_of(" "));

            int argc;
            boost::scoped_array<char*> argv(new char* [optionsArgv.size() + 2]);

            for (argc = 0; argc < (int)optionsArgv.size(); argc++) {
                DEBUG("argc: " << argc << " ; " << optionsArgv.size());
                argv[argc + 1] = const_cast<char *>(optionsArgv[argc].c_str());
            }
            argv[++argc] = 0;

            boost::program_options::store(
                boost::program_options::parse_command_line(
                    argc, argv.get(), foundReader->second->optionsHandled()),
                options);

            boost::program_options::notify(options);
        }

        return foundReader->second->createLatticeReader(options);
    }

    return NULL;
}

GuessingReader::Factory::~Factory() { }

LatticeReader<std::istream>* GuessingReader::Factory::doCreateLatticeReader(
    const boost::program_options::variables_map& options) {

    return new GuessingReader(options["block-size"].as<int>());
}

boost::program_options::options_description GuessingReader::Factory::doOptionsHandled() {
    boost::program_options::options_description optionsDescription("Allowed options");

    optionsDescription.add_options()
        ("block-size", boost::program_options::value<int>()->default_value(DEFAULT_BLOCK_SIZE),
            "the size of the input data used to determine the format");

    return optionsDescription;
}

std::string GuessingReader::Factory::doGetName() const {
    return "guessing-reader";
}

boost::filesystem::path GuessingReader::Factory::doGetFile() const {
    return __FILE__;
}

GuessingReader::Worker::Worker(GuessingReader& processor,
                               std::istream& inputStream,
                               Lattice& lattice) :
    ReaderWorker<std::istream>(inputStream, lattice),
    processor_(processor) { }

void GuessingReader::Worker::doRun() {
    std::string filetype = processor_.guessFileType(inputStream_);
    LatticeReader<std::istream>* reader = processor_.getLatticeReader(filetype);

    if (reader != NULL) {
        reader->readIntoLattice(inputStream_, lattice_);
    }
    else {
        ERROR("unknown reader for the guessed file type: " << filetype);
    }
}
