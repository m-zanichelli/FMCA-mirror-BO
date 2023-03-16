// This file is part of FMCA, the Fast Multiresolution Covariance Analysis
// package.
//
// Copyright (c) 2022, Michael Multerer
//
// All rights reserved.
//
// This source code is subject to the GNU Affero General Public License v3.0
// license and without any warranty, see <https://github.com/muchip/FMCA>
// for further information.
//
#include <Eigen/Dense>
#include <iostream>

#include "../FMCA/CovarianceKernel"
#include "../FMCA/H2Matrix"
#include "../FMCA/src/util/Tictoc.h"


#define NPTS 100000
#define DIM 2
#define MPOLE_DEG 3
#define LEAFSIZE 10

using Interpolator = FMCA::TensorProductInterpolator;
using Moments = FMCA::NystromMoments<Interpolator>;
using MatrixEvaluator = FMCA::NystromEvaluator<Moments, FMCA::CovarianceKernel>;
using H2ClusterTree = FMCA::H2ClusterTree<FMCA::ClusterTree>;
using H2Matrix = FMCA::H2Matrix<H2ClusterTree>;

int main() {
  FMCA::Tictoc T;
  const FMCA::CovarianceKernel function("EXPONENTIAL", 1);
  const FMCA::Matrix P = FMCA::Matrix::Random(DIM, NPTS);
  const Moments mom(P, MPOLE_DEG);
  H2ClusterTree ct(mom, 0, P);
  FMCA::internal::compute_cluster_bases_impl::check_transfer_matrices(ct, mom);
  const MatrixEvaluator mat_eval(mom, function);
  T.tic();
  for (FMCA::Scalar eta = 0.8; eta >= 0.1; eta *= 0.5) {
    std::cout << "eta:                          " << eta << std::endl;
    const H2Matrix hmat(ct, mat_eval, eta);
    hmat.statistics();
    {
      FMCA::Vector x(NPTS), y1(NPTS), y2(NPTS);
      FMCA::Scalar err = 0;
      FMCA::Scalar nrm = 0;
      for (auto i = 0; i < 10; ++i) {
        FMCA::Index index = rand() % P.cols();
        x.setZero();
        x(index) = 1;
        FMCA::Vector col = function.eval(P, P.col(ct.indices()[index]));
        y1 = col(ct.indices());
        y2 = hmat * x;
        err += (y1 - y2).squaredNorm();
        nrm += y1.squaredNorm();
      }
      err = sqrt(err / nrm);
      std::cout << "compression error:            " << err << std::endl;
      std::cout << std::string(60, '-') << std::endl;
    }
  }
  T.toc("elapsed time: ");
  return 0;
}
