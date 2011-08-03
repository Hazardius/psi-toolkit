
#include "rule_matcher.hpp"

#include <iostream>

namespace poleng
{

namespace bonsai
{
    namespace puddle
    {


RuleMatcher::RuleMatcher() {
//    rules = new Rules;
    syntok = true;
    disamb = false;
    norepeats = false;
}

RuleMatcher::~RuleMatcher() {
//    delete rules;
}

//void RuleMatcher::setRules(Rules aRules)
void RuleMatcher::setRules(RulesPtr aRules) {
//    delete rules;
    //rules = //new Rules(aRules);
    rules = aRules;
}

ParseGraphPtr RuleMatcher::applyRules(std::string &sentence, Entities &entities, Edges &lattice)
{
//    std::cerr << "ZDANIE!" << std::endl;
    //trzeba jechać po kolei tokenami ze zdania - zdanie to nie bedzie string, tylko obiekt: co najmniej wektor tokenow (elementow tych od grafu) oraz napis bedacy tym 'skompilowanym' ciagiem
    //dla pierwszego tokena, dla drugiego, etc, zrob: ...
    //

//    std::cerr << sentence << std::endl;

    //Entities::iterator it = entities.begin();
    //int currentEntity = 0;
    //while (1)
    //{
    bool anything = false;

    Rules::iterator ir = rules->begin();

    while (ir != rules->end())
    //for (Rules::iterator ir = rules.begin(); ir != rules.end(); ir ++)
    {
//        std::cerr << "Regula: " << (*ir)->getName() << std::endl;
        int currentMatch = 0;
        int currentEntity;
        bool first_match = true;
        std::string before = "";
        std::string prev_before = "";
        //@todo: przerobic to tak, zeby nie bylo miliard razy dopasowywane ten lancuch, tylko zeby go dopasowac tyle razy ile pasuje i odpowiednio odpalac reguly tam, gdzie pasuja
        while ((currentEntity = (*ir)->matchPattern(sentence, currentMatch, before)) > -1)
        {
            currentMatch ++;

            //OLD:
            //if (norepeats)
            //    if (prev_before == before)
            //        continue;
            if (!first_match)
            {
                if (!(*ir)->getRepeat())
                {
                    if (prev_before == before)
                        continue;
                }
            }
            first_match = false;
                    prev_before = before;
//                std::cerr << "Pasuje!" << std::endl;
//       std::cerr << "Zdanie: " << sentence << std::endl;
        bool structureChanged = false;
        //int currentEntity = 0;
        //for (Entities::iterator it = entities.begin(); it != entities.end(); it ++)
       // {
        std::string oldSentence = sentence;
            if ((*ir)->test(sentence, entities, currentEntity))
            {
//                std::cerr << "test zdany" << std::endl;
                if ((*ir)->apply(sentence, entities, lattice, currentEntity))
                {
                    sentence = generateSentencePattern(lattice);
                    std::cerr << "SENT PRZED: " << oldSentence << std::endl;
                    std::cerr << "SENT PO:    " << sentence << std::endl;
                    //TU: na nowo zrobic sentence
//                    prev_before = before;
//                    std::cerr << "zaaplikowany" << std::endl;
                    if (sentence != oldSentence)
                        currentMatch --;
                    structureChanged = true;
              //      break;
                }
            }

        //    currentEntity ++;
        //}

        if (structureChanged)
        {
        //    ir = rules.begin();
        //    currentMatch = 0;
            continue;
        }
        }
        ir ++;
    }

    unescapeSpecialChars(lattice);

    addPosEdges(lattice);

    for (Edges::iterator e = lattice.begin(); e != lattice.end(); e++)
    {
        if (!(*e)->isLexical())
        //if ((*e)->isPhrasal())
            continue;

        bool lowFlag = false;
        std::string id = (*e)->getId();
        int start = (*e)->getStart();
        int end = (*e)->getEnd();

        std::string lowcase = (*e)->getLabel();
        boost::to_lower(lowcase);
//        UnicodeString valL = icuConverter::fromUtf((*e)->getLabel());
//        valL.toLower();
//        StringCharacterIterator itL(valL);
//        std::stringstream ss;
//        while (itL.hasNext())
//        {
//            UnicodeString tl = itL.current();
//            ss << icuConverter::toUtf(tl);
//            itL.next();
//        }
//        lowcase = ss.str();
        if (lowcase != (*e)->getLabel())
            lowFlag = true;

        if (lowFlag)
        {
            for (int vari = 0; vari < (*e)->variants_.size(); vari ++)
            {
                std::string var_base = boost::get<0>((*e)->variants_[vari]);

                std::string lowbase = var_base;
                boost::to_lower(lowbase);
//                UnicodeString valL = icuConverter::fromUtf(var_base);
//                valL.toLower();
//                StringCharacterIterator itL(valL);
//                std::stringstream ss;
//                while (itL.hasNext())
//                {
//                    UnicodeString tl = itL.current();
//                    ss << icuConverter::toUtf(tl);
//                    itL.next();
//                }
//                lowbase = ss.str();
                if (lowbase == var_base) //istnieje forma bazowa pisana mala litera
                {
                    (*e)->setLabel(lowcase);
                    break;
                }
            }
        }

    }

    ParseGraphPtr pg = ParseGraphPtr(new ParseGraph());

    if (!syntok)
    {
        std::map<int, int> begins_mapped;
        for (Edges::iterator e = lattice.begin(); e != lattice.end(); e ++)
        {
                if ((*e)->getType() == "group" && (*e)->getLabel() == "SYNTOK")
                {
                    std::map<int, int>::iterator it = begins_mapped.find((*e)->getStart());
                    if (it != begins_mapped.end())
                    {
                        begins_mapped[(*e)->getStart()] = it->second + 1;
                    }
                    else
                    {
                        begins_mapped.insert(std::pair<int, int>((*e)->getStart(), 1));
                    }
                    //if (((*e)->getEnd() - (*e)->getStart()) > 1)
                    //{
                    //e ++;
                    //}
                }
        }

        for (Edges::iterator e = lattice.begin(); e != lattice.end(); e ++)
        {
            if ((*e)->getType() == "group" && (*e)->getLabel() == "SYNTOK")
                continue;
            std::map<int, int>::iterator it = begins_mapped.find((*e)->getStart());
            if (it != begins_mapped.end())
            {
                (*e)->setDepth((*e)->getDepth() - it->second);
            }

            pg->add_edge((*e)->getStart(), (*e)->getEnd(), **e);
        }
    }
    else
    {
        for (Edges::iterator e = lattice.begin(); e != lattice.end(); e ++)
            pg->add_edge((*e)->getStart(), (*e)->getEnd(), **e);
    }
//    e ++;

    return pg;
    //if (!anything)
    //    break;
    //}
}

void RuleMatcher::unescapeSpecialChars(Edges &edges) {
    for (Edges::iterator e = edges.begin(); e != edges.end(); e++) {
        if (!(*e)->isLexical())
            continue;

        std::string label = (*e)->getLabel();
        label = util::unescapeSpecialChars(label);
        (*e)->setLabel(label);

        for (int vari = 0; vari < (*e)->variants_.size(); vari ++)
        {
            std::string var_base = boost::get<0>((*e)->variants_[vari]);
            var_base = util::unescapeSpecialChars(var_base);
            if (var_base != boost::get<0>((*e)->variants_[vari]))
            {
                PosInfo new_var = PosInfo(var_base, boost::get<1>((*e)->variants_[vari]), boost::get<2>((*e)->variants_[vari]));
                (*e)->variants_[vari] = new_var;
            }
        }
    }
}

void RuleMatcher::addPosEdges(Edges &edges) {
    Edges posEdges;

    for (Edges::iterator e = edges.begin(); e != edges.end(); e++) {
        if (!(*e)->isLexical())
            continue;

        int start = (*e)->getStart();
        int end = (*e)->getEnd();

        std::set<std::string> parts;

        std::vector<PosInfo> vars = (*e)->variants_;
        for (std::vector<PosInfo>::iterator pi = (*e)->variants_.begin(); pi != (*e)->variants_.end(); pi ++)
        {
            if (!boost::get<2>(*pi))
                continue;
            std::string morpho = boost::get<1>(*pi);
            std::string part = morpho.substr(0, morpho.find(":", 1));
            if (parts.find(part) == parts.end())
            {
                TransitionInfo *pos = new TransitionInfo("pos");
                pos->setLabel(part);
                pos->setDepth(1);
                pos->setStart(start);
                pos->setEnd(end);
                pos->variants_ = (*e)->variants_;
                pos->setHead((*e)->getId());

                std::stringstream *ss = new std::stringstream;
                *ss << std::hex << Group::groupId;
                std::string pos_id = ss->str();
                delete ss;
                pos->setId(pos_id);
                Group::groupId++;

                posEdges.push_back(pos);
                parts.insert(part);
                if (!disamb)
                    break;
            }
        }
        if (parts.size() == 0) //gdy nie bylo zadnych interpretacji, bo wyrznely wszystko reguly
        {
            if ((*e)->variants_.size() == 0)
            {
                (*e)->addMorphology(PosInfo((*e)->getLabel(), "ign", 1));
            }
            PosInfo pi = (*e)->variants_.front();
            std::string base = boost::get<0>(pi);
            (*e)->addMorphology(PosInfo(base, "ign", 1));
            TransitionInfo *pos = new TransitionInfo("pos");
            pos->setLabel("ign");
            pos->setDepth(1);
            pos->setStart((*e)->getStart());
            pos->setEnd((*e)->getEnd());
            pos->setHead((*e)->getId());
            pos->variants_ = (*e)->variants_;

            posEdges.push_back(pos);
        }
    }

    for (Edges::iterator ei = posEdges.begin(); ei != posEdges.end(); ei++)
        edges.push_back(*ei);

}

std::string RuleMatcher::generateSentencePattern(Edges &edges) {
    ParseGraphPtr tmp_pg = ParseGraphPtr(new ParseGraph()); //tymczasowy graf, wszak dzialamy docelowo na grafie/kracie, a nie na jakis badziewiach
    for (Edges::iterator e = edges.begin(); e != edges.end(); e ++) {
        tmp_pg->add_edge((*e)->getStart(), (*e)->getEnd(), **e);
    }
    std::stringstream ss;
    ss << "<<s<0<sb<>";

    ParseGraph::Graph *g = tmp_pg->getBoostGraph();
    //ParseGraph::Vertex end = vertex(boost::num_vertices(*g) - 1, *g); //potrzebne to w ogole?
    ParseGraph::VertexIndex index = get(boost::vertex_index, *g);
    ParseGraph::TransitionMap map = get(boost::edge_bundle, *g);

    std::string emptyMorphology(tagset->getNumberOfAttributes(), '0');
    int i = 0;
    while (i < (boost::num_vertices(*g) - 1) ) {
    //for (int i = 0; i < boost::num_vertices(*g); i ++) {
        ParseGraph::Vertex v = vertex(i, *g);

        int max_depth = 0;
        int max_start, max_end;
        std::string max_label, max_type, max_orth;
        std::vector<PosInfo> max_variants;

        for (std::pair <ParseGraph::OutEdgeIt, ParseGraph::OutEdgeIt> p = out_edges(v, *g); p.first != p.second; ++ p.first) {
            ParseGraph::Edge e = *p.first;
            if (map[e].isPos())
                continue;
            if ((max_depth == 0) || ((max_depth > 0) && (map[e].getDepth() > max_depth))) {
                max_depth = map[e].getDepth();
                max_start = map[e].getStart();
                max_end = map[e].getEnd();
                max_label = map[e].getLabel();
                max_type = map[e].getType();
                max_orth = map[e].getOrth();
                max_variants = map[e].variants_;
            }
        }

        if (max_type != "group") {
            ss << "<<t";
        } else {
            ss << "<<g";
        }
        ss << "<" << max_start;
        ss << "<" << max_end;
        if (max_type == "group") {
            ss << "<" << max_label;
        } else {
            ss << "<" << "TOKEN";
        }
        ss << "<" << max_orth; //@todo: nie dziala to dla gruyp
        if (max_variants.size() == 0) {
            PosInfo pi(max_orth, "ign", 1);
            max_variants.push_back(pi);
        }
        for (std::vector<PosInfo>::iterator vit = max_variants.begin();
                vit != max_variants.end(); vit ++) {
            if (! boost::get<2>(*vit))
                continue;
            std::string mapped = tagset->mapMorphology(boost::get<1>(*vit));
            if (mapped == "")
                mapped = emptyMorphology;
            ss << "<";
            ss << mapped;
            ss << boost::get<0>(*vit);
        }
        ss << ">";
        i = max_end;
    }
    ss << "<<s<666<se<>";

    return ss.str();
}

void RuleMatcher::setSyntok() {
    syntok = true;
}

void RuleMatcher::setNoSyntok() {
    syntok = false;
}

void RuleMatcher::setDisamb() {
    disamb = true;
}

void RuleMatcher::setNoRepeats() {
    norepeats = true;
}

}

}

}
