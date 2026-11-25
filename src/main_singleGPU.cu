#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cuda_runtime.h>

__global__ void vectorAdd(const float* a, const float* b, float* c, int n){
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < n) c[i] = a[i] + b[i];
}

#define CUDA_CHECK(call)                                              \
  do {                                                                \
    cudaError_t err = (call);                                         \
    if (err != cudaSuccess) {                                         \
      std::cout << "CUDA error at " << __FILE__ << ":" << __LINE__    \
                << " -> " << cudaGetErrorString(err) << '\n';         \
      std::exit(1);                                                   \
    }                                                                 \
  } while (0)

int main (){
  const int n = 1 << 20;
  const size_t bytes = n * sizeof(float);
  cudaDeviceProp prop{};
  CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
  std::cout << "GPU: " << prop.name << " (sm_" << prop.major << prop.minor
            << ")" << '\n';
  std::vector<float> h_a(n, 1.0f), h_b(n, 2.0f), h_c(n, 0.0f);
  float *d_a, *d_b, *d_c;
  CUDA_CHECK(cudaMalloc(&d_a, bytes));
  CUDA_CHECK(cudaMalloc(&d_b, bytes));
  CUDA_CHECK(cudaMalloc(&d_c, bytes));

  CUDA_CHECK(cudaMemcpy(d_a, h_a.data(), bytes, cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(d_b, h_b.data(), bytes, cudaMemcpyHostToDevice));

  const int threads = 256;
  const int blocks = (n + threads - 1) / threads;
  std::cout << "n=" << n << ", " << threads << " threads per block, " << blocks
            << " blocks\n";

  vectorAdd<<<blocks, threads>>>(d_a, d_b, d_c, n);
  CUDA_CHECK(cudaGetLastError());
  CUDA_CHECK(cudaDeviceSynchronize());
  CUDA_CHECK(cudaMemcpy(h_c.data(), d_c, bytes, cudaMemcpyDeviceToHost));

  bool ok = true;
  for (int i = 0; i < n; ++i) {
    if (h_c[i] != 3.0f) {
      ok = false;
      break;
    }
  }
  printf("c[0] = %f, c[n-1] = %f -> %s\n", h_c[0], h_c[n-1],
          ok ? "all correct (3.0)" : "Wrong result");
  CUDA_CHECK(cudaFree(d_a));
  CUDA_CHECK(cudaFree(d_b));
  CUDA_CHECK(cudaFree(d_c));
  return 0;
}