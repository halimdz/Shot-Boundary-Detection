#include "softcut_detection.hpp"
#ifndef _WIN32
#include <glog/logging.h>
#endif
#include <src/soft_cut/classification/caffe_classifier.hpp>
#include "src/soft_cut/io/file_writer.hpp"
#include <boost/format.hpp>
#include <src/soft_cut/io/file_reader.hpp>

void wrongUsage();

using namespace sbd;

int SoftCutMain::main(po::variables_map flagArgs, std::map<std::string, std::string> inputArguments) {
    // Disable logging (1: log warnings, 3: log nothing)
#ifndef _WIN32
    FLAGS_minloglevel = 1;

    // Caffe parameters
    std::string preModel = "/home/pva_t1/Shot-Boundary-Detection/nets/snapshots/_iter_110000.caffemodel";
    std::string protoFile = "/home/pva_t1/Shot-Boundary-Detection/nets/deploy.prototxt";

    bool cpuSetting = false;
    cv::Size size(227, 227);
    int channels = 3;
    bool isDebug = true;
    std::string resultLayer = "argmax";
    std::string dataLayer = "data";
    int batchSize = 70;
    int nrClasses = 2;

    // programm parameters
    int sequenceSize = 10;
    int sequenceBatchSize = batchSize / sequenceSize;
    std::string txtFile = "/opt/data_sets/video_sbd_dataset/frames/test_test.txt"; // TODO adapt to correct file
    std::string outputFile = "/home/pva_t1/Shot-Boundary-Detection/resources/predictions.txt";

    /**
    * MAIN
    */

    // TODO
    // (1) Die Sequenzen werden pro Video erzeugt, aber zusammen in den Sequenze-Vektor gespeichert.
    //     Eine Sequenze-Batch darf nicht über Videos hinaus gehen.
    // (2) Wir müsen alle Sequenzen von einem Video bestimmen!
    // (3) Evaluierung einbauen: Frame-Level, Sequence-Level, Video-Level
    // (4) Mergen von 10er Sequenzen die einen Soft-Cut darstellen. -> Macht Felix
    //     Beispiel: 5 Sequenzen wurden als Soft Cut erkannt, eine nicht und dann wieder 5 als Soft Cut.
    //               Die 11 Sequenzen sind wahrscheinlich ein Soft Cut.


    // 1. Get all sequences with frame paths and class label
    std::vector<Sequence> sequences;
    FileReader::load(txtFile, sequenceSize, sequences);

    if (sequences.size() % sequenceBatchSize != 0) {
        std::cout << "Number of sequences (" << sequences.size() << ") modulo " << sequenceBatchSize << " is not equal to 0!" << std::endl;
        exit(99);
    }

    // 2. Initialize classifier
    std::cout << "Initialize classifier ..." << std::endl;
    CaffeClassifier classifier(cpuSetting, preModel, protoFile, size, channels, isDebug);

    FileWriter writer(outputFile);

    // 4. Predict all sequences
    std::cout << "Predicting " << sequences.size() << " sequences ..." << std::endl;
    for (int i = 0; i < sequences.size(); i += sequenceBatchSize) {
        std::cout << (i * 100) / sequences.size() << "% " << std::flush;

        // get data for the batch of sequences
        SequenceBatch sequenceBatch = getSequenceBatch(sequences, i, sequenceBatchSize);

        // get prediction for frames
        std::vector<float> predictions;
        classifier.predict(sequenceBatch.frames, sequenceBatch.labels, resultLayer, dataLayer, predictions);

        // write predictions
        writePrediction(sequences, predictions, i, sequenceSize, writer);

        predictions.clear();
    }
    std::cout << std::endl;

    std::cout << "Wrote prediction to " << outputFile << std::endl;
    writer.close();
#endif
    return 0;
}


void SoftCutMain::writePrediction(std::vector<Sequence> sequences,
    std::vector<float> predictions,
    int i, int sequenceSize,
    FileWriter &writer) {

    for (int k = 0; k < predictions.size(); k++) {
        Sequence sequence = sequences[i + k / sequenceSize];

        int pred = (int)predictions[k];
        int actual = sequence.clazz;

        boost::format line("Frame: %s Predicted: %-3d Actual: %-3d");
        line % sequence.frames[k % sequenceSize];
        line % pred;
        line % actual;
        writer.writeLine(line.str());
    }
}

SequenceBatch SoftCutMain::getSequenceBatch(std::vector<Sequence> sequences, int start, int nrSequences) {
    std::vector<cv::Mat> frames;
    std::vector<int> labels;

    for (int i = start; i < start + nrSequences; i++) {
        Sequence sequence = sequences[i];

        // reading frames and labels of sequence
        for (int j = 0; j < sequence.frames.size(); j++) {
            std::string frameFile = sequence.frames[j];
            cv::Mat frame = cv::imread(frameFile);

            // check if image contains data
            if (!frame.data) {
                std::cerr << "Warning: input image (" << frameFile << ") for caffe classifier is empty." << std::endl;
                exit(1);
            }

            frames.push_back(frame);
            labels.push_back(sequence.clazz);
        }
    }

    SequenceBatch sequenceBatch;
    sequenceBatch.frames = frames;
    sequenceBatch.labels = labels;
    return sequenceBatch;
}

void wrongUsageSoftCut()
{
    std::cout << "Usage: sbd --soft_cut" << std::endl;
#ifdef _WIN32
    system("pause");
#else
    cv::waitKey(0);
#endif
    exit(1);
}