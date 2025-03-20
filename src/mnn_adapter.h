//
// Created by liangtao_deng on 2025/3/17.
//

#ifndef NKF_MNN_DEPLOY_R328_MNN_ADAPTER_H
#define NKF_MNN_DEPLOY_R328_MNN_ADAPTER_H

#include <MNN/ImageProcess.hpp>
#include <MNN/Interpreter.hpp>
#include <MNN/MNNDefine.h>
#include <MNN/Tensor.hpp>
#include <MNN/MNNForwardType.h>

#include <iostream>
#include <fstream>
#include <vector>
#include "../model_loader/model_loader.h"

class MNNAudioAdapter {
public:

    MNNAudioAdapter(const std::string &model, int thread, bool use_model_bin = false) {
        backend_ = MNN_FORWARD_CPU;
        detect_model_ = std::shared_ptr<MNN::Interpreter>(
                MNN::Interpreter::createFromFile(model.c_str()));

        _config.type = backend_;
        _config.numThread = thread;
        MNN::BackendConfig backendConfig;
        backendConfig.precision = MNN::BackendConfig::Precision_High;
        backendConfig.power = MNN::BackendConfig::Power_High;
        _config.backendConfig = &backendConfig;

    }

    MNNAudioAdapter(const solex::Model *model, int thread) {
        backend_ = MNN_FORWARD_CPU;
        detect_model_ =
                std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromBuffer(
                        model->caffemodelBuffer, model->modelsize.caffemodel_size));
        _config.type = backend_;
        _config.numThread = thread;

    }


    ~MNNAudioAdapter() {
        detect_model_->releaseModel();
        detect_model_->releaseSession(sess);
    }

    void Init(const std::string &input, int numcp,int frame_num) {
        sess = detect_model_->createSession(_config);
    }

    std::vector<float> Infer(const std::vector<float>& nearEnd, const std::vector<float>& farEnd) {
        auto inputTensor =detect_model_->getSessionInput(sess, nullptr);
        auto farTensor = detect_model_->getSessionInput(sess, "farEnd");
        // 复制 nearEnd 数据到 inputTensor
        memcpy(inputTensor->host<float>(), nearEnd.data(), nearEnd.size() * sizeof(float));
        memcpy(farTensor->host<float>(), farEnd.data(), farEnd.size() * sizeof(float));

        // 运行 AEC 计算
        detect_model_->runSession(sess);

        // 获取去除回声后的输出
        auto outputTensor = detect_model_->getSessionOutput(sess, nullptr);
        std::vector<float> processedAudio(nearEnd.size());
        memcpy(processedAudio.data(), outputTensor->host<float>(), processedAudio.size() * sizeof(float));

        return processedAudio;
    }


private:
    std::shared_ptr<MNN::Interpreter> detect_model_;
    MNN::Session *sess{};
    MNN::ScheduleConfig _config;
    MNNForwardType backend_;

};


#endif //NKF_MNN_DEPLOY_R328_MNN_ADAPTER_H
