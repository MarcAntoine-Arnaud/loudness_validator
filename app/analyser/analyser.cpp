#include <string>
#include <iostream>
#include <cstring>
#include <vector>
#include <ctime>

#include <loudnessAnalyser/LoudnessAnalyser.hpp>

#include <loudnessIO/ProcessFile.hpp>
#include <loudnessIO/SoundFile.hpp>
#include <loudnessTools/WriteXml.hpp>

bool showProgress = false;
bool showResults = false;

void progress(int p)
{
    if(showProgress)
        std::cout << "[" << p << "%]\r" << std::flush;
}

int main(int argc, char** argv)
{
    bool validToProcess = false;
    bool showTime = false;
    bool enableOptimization = true;
    std::vector<int> standards;
    std::vector<std::string> filenames;

    time_t start, end;

    for(int i = 1; i < argc; i++)
    {

        if(strcmp(argv[i], "--progress") == 0)
        {
            showProgress = true;
        }
        if(strcmp(argv[i], "--verbose") == 0)
        {
            showProgress = true;
            showResults = true;
        }
        if(strcmp(argv[i], "--time") == 0)
        {
            showTime = true;
        }
        if(strcmp(argv[i], "--disable-optimization") == 0)
        {
            enableOptimization = false;
        }
        if(strncmp(argv[i], "--standard=", 11) == 0)
        {
            if(strcmp(argv[i], "--standard=cst") == 0)
            {
                standards.push_back(0);
            }
            else
            {
                if(strcmp(argv[i], "--standard=ebu") == 0)
                {
                    standards.push_back(1);
                }
                else
                {
                    if(strcmp(argv[i], "--standard=atsc") == 0)
                    {
                        standards.push_back(2);
                    }
                    else
                    {
                        std::cout << "Error: unknown standard specified in command line" << std::endl;
                        return -100;
                    }
                }
            }
        }
        if(standards.empty())
        {
            standards.push_back(1); // default standard : EBU R128
        }
        std::string ext(argv[i]);
        ext.erase(0, ext.length() - 5);
        std::string ext4 = ext;
        ext4.erase(0, 1);
        if(strcmp(ext.c_str(), ".aiff") == 0 || strcmp(ext4.c_str(), ".aif") == 0 || strcmp(ext4.c_str(), ".wav") == 0)
        {
            filenames.push_back(argv[i]);
            validToProcess = true;
        }
    }
    if(validToProcess)
    {
        for(size_t i = 0; i < filenames.size(); i++)
        {
            std::cout << filenames.at(i) << std::endl;

            std::string filename(filenames.at(i));
            if(filename.at(filename.length() - 4) == '.')
                filename.erase(filename.length() - 4, 4);
            else if(filename.at(filename.length() - 5) == '.')
                filename.erase(filename.length() - 5, 5);

            filename.append("_PLoud.xml");
            Loudness::tools::WriteXml writerXml(filename, filenames.at(i));

            for(size_t j = 0; j < standards.size(); j++)
            {
                Loudness::io::SoundFile audioFile;
                Loudness::analyser::LoudnessLevels levels =
                    standards.at(j) == 0 ? Loudness::analyser::LoudnessLevels::Loudness_CST_R017()
                                         : standards.at(j) == 1 ? Loudness::analyser::LoudnessLevels::Loudness_EBU_R128()
                                                                : Loudness::analyser::LoudnessLevels::Loudness_ATSC_A85();

                Loudness::analyser::LoudnessAnalyser loudness(levels);
                if(!audioFile.open_read(filenames.at(i).c_str()))
                {
                    time(&start);
                    Loudness::io::AnalyseFile analyser(loudness, audioFile);
                    analyser.enableOptimization(enableOptimization);
                    analyser(progress);
                    time(&end);
                    if(showResults)
                        loudness.printPloudValues();
                    audioFile.close();
                    writerXml.writeResults("unknown", loudness);
                    double dif = difftime(end, start);
                    if(showTime)
                        std::cout << "processing time: " << dif << " seconds." << std::endl;
                }
            }
        }
    }
    else
    {
        std::cout << "Loudness Analyser" << std::endl;
        std::cout << "Author: Marc-Antoine ARNAUD" << std::endl << std::endl;
        std::cout << "Common usage :" << std::endl;
        std::cout << "\tloudness-analyser [options] filename.ext" << std::endl << std::endl;
        std::cout << "Options :" << std::endl;
        std::cout << "\t--standard=ebu/cst/atsc : select one standard to validate the Loudness" << std::endl;
        std::cout << "\t\t\tebu:  EBU R 128 (default)" << std::endl;
        std::cout << "\t\t\tcst:  CST RT 017" << std::endl;
        std::cout << "\t\t\tatsc: ATSC A/85" << std::endl;
        return -1;
    }
    return 0;
}
