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

#include "gather.cu.h"
#include "paddle/framework/eigen.h"
#include "paddle/operators/gather_op.h"
#include "scatter.cu.h"

namespace paddle {
namespace operators {

// template <typename T>
__global__ void print_arr(const float *params, const int N) {
  CUDA_1D_KERNEL_LOOP(i, N) { printf("device: %d, %f\n", i, params[i]); }
}

template <typename T>
class GatherOpCUDAKernel : public framework::OpKernel {
 public:
  void Compute(const framework::ExecutionContext &ctx) const override {
    PADDLE_ENFORCE(platform::is_gpu_place(ctx.GetPlace()),
                   "This kernel only runs on GPU device.");
    auto *x = ctx.Input<Tensor>("X");
    auto *index = ctx.Input<Tensor>("Index");
    auto *output = ctx.Output<Tensor>("Out");

    output->mutable_data<T>(ctx.GetPlace());

    GPUTGather<T>(ctx.GetPlace(), x, index, output);
  }
};

template <typename T>
class GatherGradOpCUDAKernel : public framework::OpKernel {
 public:
  void Compute(const framework::ExecutionContext &ctx) const override {
    PADDLE_ENFORCE(platform::is_gpu_place(ctx.GetPlace()),
                   "This kernel only runs on GPU device.");
    LOG(INFO) << "Gather grad here";
    auto *Index = ctx.Input<Tensor>("Index");
    auto *dX = ctx.Output<Tensor>(framework::GradVarName("X"));
    auto *dO = ctx.Input<Tensor>(framework::GradVarName("Out"));
    auto *x = ctx.Input<Tensor>("X");

    dX->mutable_data<T>(ctx.GetPlace());
    auto dxt = framework::EigenVector<T>::Flatten(*dX);
    auto place = ctx.GetEigenDevice<platform::GPUPlace>();
    dxt.device(place) = dxt.constant(static_cast<T>(0));

    GPUTScatter<T>(ctx.GetPlace(), dO, Index, dX);
  }
};

}  // namespace operators
}  // namespace paddle

namespace ops = paddle::operators;
REGISTER_OP_GPU_KERNEL(gather, ops::GatherOpCUDAKernel<float>);
REGISTER_OP_GPU_KERNEL(gather_grad, ops::GatherGradOpCUDAKernel<float>);
