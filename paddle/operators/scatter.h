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
#include <cstring>

#include "paddle/framework/ddim.h"
#include "paddle/framework/eigen.h"
#include "paddle/framework/tensor.h"
#include "paddle/platform/place.h"

namespace paddle {
namespace operators {

using Tensor = framework::Tensor;

// Implementation of CPU copy
template <typename T>
void CPUScatterAssign(const T* src, const int* index, const int slice_size,
                      const int index_size, T* output) {
  // paddle::framework::DDim output_dims = output->dims();
  const size_t slice_bytes = slice_size * sizeof(T);

  for (int i = 0; i < index_size; ++i) {
    int index_ = index[i];
    memcpy(output + index_ * slice_size, src + i * slice_size, slice_bytes);
  }
}

/**
 * Return a updated tensor from source tensor, scattered according to index:
 * dst[i] = src[index[i]]
 * input[src]: type-T source Tensor
 * input[index]: type-int index Tensor (1-D)
 * return: output tensor
 */
template <typename T>
void ScatterAssign(const platform::Place& place,
                   const paddle::framework::Tensor* src,
                   const paddle::framework::Tensor* index,
                   paddle::framework::Tensor* output) {
  PADDLE_ENFORCE(platform::is_cpu_place(place));
  // check index of shape 1-D
  PADDLE_ENFORCE(index->dims().size() == 1);
  int index_size = index->dims()[0];

  auto src_dims = src->dims();
  auto dst_dims = output->dims();

  const T* p_src = src->data<T>();
  const int* p_index = index->data<int>();
  T* p_output = output->data<T>();

  // check src shape and dst shape should match
  for (int i = 1; i < src_dims.size(); i++)
    PADDLE_ENFORCE(src_dims[i] == dst_dims[i]);

  // slice size
  size_t slice_size = 1;
  for (int i = 1; i < src_dims.size(); ++i) slice_size *= src_dims[i];

  CPUScatterAssign<T>(p_src, p_index, slice_size, index_size, p_output);
}

}  // namespace operators
}  // namespace paddle
