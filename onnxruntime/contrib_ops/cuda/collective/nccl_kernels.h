// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "core/providers/cuda/cuda_kernel.h"

#if defined(ORT_USE_NCCL) || defined(ORT_USE_MPI)
#ifndef USE_ROCM
#include "custom_reduce_impl.h"
#include "ipc_utils.h"
#endif
#endif

#if defined(ORT_USE_NCCL)
#include <algorithm>
#include <optional>
#include <tuple>
#include <nccl.h>
#include <sstream>
#include <string>
#endif

namespace onnxruntime {
namespace contrib {
namespace cuda {

#define NCCL_RETURN_IF_ERROR(expr) ORT_RETURN_IF_ERROR(NCCL_CALL(expr))

ncclDataType_t GetNcclDataType(onnxruntime::MLDataType type);

// -----------------------------------------------------------------------
// Defines a new version of nccl classes
// that independent with training::DistributedRunContext, only rely on MPI
// -----------------------------------------------------------------------
class NcclContext final {
 public:
  NcclContext();
  ~NcclContext();

  ncclComm_t Comm() {
    return comm_;
  }

  int Rank() const {
    return rank_;
  }

  int Size() const {
    return world_size_;
  }

 private:
  ncclComm_t comm_;
  int rank_;
  int world_size_;
};

class NcclKernel : public ::onnxruntime::cuda::CudaKernel {
 public:
  explicit NcclKernel(const OpKernelInfo& info);

  ncclComm_t Comm() const {
    return nccl_->Comm();
  }

 protected:
  NcclContext* nccl_ = nullptr;
};

/*
 * Defines new version of Nccl classes that independent with training::DistributedContext
 * only rely on MPI
 */
class AllReduce final : public NcclKernel {
 public:
  explicit AllReduce(const OpKernelInfo& info);

  Status ComputeInternal(OpKernelContext* context) const override;
};

class AllGather final : public NcclKernel {
 public:
  explicit AllGather(const OpKernelInfo& info);

  Status ComputeInternal(OpKernelContext* context) const override;

 private:
  int64_t group_size_ = -1;
  int64_t axis_ = -1;
  const CUDAExecutionProvider* cuda_ep_;
};

class AllToAll final : public NcclKernel {
 public:
  explicit AllToAll(const OpKernelInfo& info);

  Status ComputeInternal(OpKernelContext* context) const override;

 private:
  int64_t group_size_ = -1;
};

Status FuncAllReduce(
    ncclComm_t comm,
    cudaStream_t stream,
    const Tensor* input,
    Tensor* output);

#ifndef USE_ROCM
Status FuncCustomAllReduce(
    NcclContext* nccl,
    cudaStream_t stream,
    const void* input_data,
    void* output_data,
    int64_t input_count,
    onnxruntime::MLDataType data_type,
    onnxruntime::cuda::collective::IPCMemoryResourcePack& ipc_mem_res_pack);
#endif

void FuncAllGather(
    const NcclKernel* nccl_kernel,
    OpKernelContext* ctx,
    const Tensor* input,
    const int64_t group_size,
    const int64_t axis,
    Tensor* output);

std::unique_ptr<Tensor> FuncAllGather(
    const NcclKernel* nccl_kernel,
    OpKernelContext* ctx,
    const Tensor* input,
    const int64_t group_size,
    const int64_t axis);

}  // namespace cuda
}  // namespace contrib
}  // namespace onnxruntime
