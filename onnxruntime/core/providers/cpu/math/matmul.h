// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "core/framework/op_kernel.h"
#include "core/util/sparse_cpu_util.h"

namespace onnxruntime {

template <typename T>
struct MatMulSparseInfo {
  sparse_util::SparseMatrixRow<T> sparse_initializer_;
  TensorShape shape_;
};

template <typename T>
class MatMul final : public OpKernel {
 public:
  MatMul(const OpKernelInfo& info) : OpKernel(info) {}

  Status PrePack(const Tensor& tensor, const PrepackParam&, bool& is_packed) override;

  Status Compute(OpKernelContext* context) const override;

 private:
  std::unique_ptr<MatMulSparseInfo<T>> sparse_info_;
};

template <>
class MatMul<float> final : public OpKernel {
 public:
  MatMul(const OpKernelInfo& info) : OpKernel(info) {
    info.GetAttrOrDefault<int64_t>("transA", &trans_a_attr_, 0);
    info.GetAttrOrDefault<int64_t>("transB", &trans_b_attr_, 0);
    info.GetAttrOrDefault<float>("alpha", &alpha_attr_, 1.0);
  }

  Status PrePack(const Tensor& tensor, const PrepackParam&, bool& is_packed) override;

  Status Compute(OpKernelContext* context) const override;

 private:
  TensorShape b_shape_;
  BufferUniquePtr packed_b_;

  // For FusedMatMul contrib ops
  float alpha_attr_;
  int64_t trans_a_attr_;
  int64_t trans_b_attr_;
  std::unique_ptr<MatMulSparseInfo<float>> sparse_info_;
};

}  // namespace onnxruntime
