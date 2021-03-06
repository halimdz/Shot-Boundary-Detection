#pragma once

#include "../forwarddeclarations.hpp"

namespace sbd {

    class TransitionGenerator {
    private:
        std::vector<sbd::GoldStandardElement> m_gold;
        std::string m_dataFolder;
        std::vector<std::string> m_tweenerNames;
        std::ofstream m_filesTxtOut;
        std::string getDatasetName();
    public:
        TransitionGenerator(std::unordered_set<sbd::GoldStandardElement> &gold, std::string dataFolder);
        int createRandomTransition();
        void createRandomTransitions(int amount);
        int writeFilesTxtForTestData();
    };

}
