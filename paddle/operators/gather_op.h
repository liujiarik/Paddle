/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserve.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#pragma once
#include "gather.h"
#include "paddle/framework/eigen.h"
#include "paddle/framework/op_registry.h"
#include "scatter.h"

namespace paddle {
namespace operators {

using Tensor = framework::Tensor;

template <typename T>
class GatherOpKernel : public framework::OpKernel<T> {
 public:
  void Compute(const framework::ExecutionContext &ctx) const override {
    PADDLE_ENFORCE(platform::is_cpu_place(ctx.GetPlace()),
                   "This kernel only runs on CPU.");

    auto *x = ctx.Input<Tensor>("X");
    auto *index = ctx.Input<Tensor>("Index");
    auto *output = ctx.Output<Tensor>("Out");

    output->mutable_data<T>(ctx.GetPlace());

    CPUTGather<T>(ctx.GetPlace(), x, index, output);
  }
};

template <typename T>
class GatherGradientOpKernel : public framework::OpKernel<T> {
 public:
  void Compute(const framework::ExecutionContext &ctx) const override {
    PADDLE_ENFORCE(platform::is_cpu_place(ctx.GetPlace()),
                   "This kernel only runs on CPU.");

    auto *Index = ctx.Input<Tensor>("Index");
    auto *dX = ctx.Output<Tensor>(framework::GradVarName("X"));
    auto *dO = ctx.Input<Tensor>(framework::GradVarName("Out"));

    dX->mutable_data<T>(ctx.GetPlace());
    auto dxt = framework::EigenVector<T>::Flatten(*dX);
    auto place = ctx.GetEigenDevice<platform::CPUPlace>();
    dxt.device(place) = dxt.constant(static_cast<T>(0));

    ScatterAssign<T>(ctx.GetPlace(), dO, Index, dX);
  }
};

}  // namespace operators
}  // namespace paddle
