// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G. Grisetti, W. Burgard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef EDGE_ROOM_PLANE_HPP
#define EDGE_ROOM_PLANE_HPP

#include <Eigen/Dense>
#include <g2o/core/base_binary_edge.h>
#include <g2o/types/slam3d_addons/vertex_plane.h>
#include <g2o/types/slam3d/vertex_se3.h>
#include "g2o/vertex_room.hpp"
namespace g2o {

class EdgeSE3Room : public BaseBinaryEdge<2, Eigen::Vector2d, g2o::VertexSE3, g2o::VertexRoomXYLB> {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  EdgeSE3Room() : BaseBinaryEdge<2, Eigen::Vector2d, g2o::VertexSE3, g2o::VertexRoomXYLB>() {
    _information.setIdentity();
    _error.setZero();
  }

  void computeError() override {
    const VertexSE3* v1 = static_cast<const VertexSE3*>(_vertices[0]);
    const VertexRoomXYLB* v2 = static_cast<const VertexRoomXYLB*>(_vertices[1]);

    Eigen::Isometry3d m2l = v1->estimate().inverse();
    Eigen::Isometry3d room_map; room_map.matrix().block<4,4>(0,0) = Eigen::Matrix4d::Identity(); room_map.matrix().block<2,1>(0,3) = v2->estimate();

    Eigen::Isometry3d room_local = room_map * m2l; 
    Eigen::Vector2d est = room_local.matrix().block<2,1>(0,3);
    _error = est - _measurement; 
   }

 virtual bool read(std::istream& is) override {
    Eigen::Vector2d v;
    is >> v(0) >> v(1);

    setMeasurement(v);
    for(int i = 0; i < information().rows(); ++i) {
      for(int j = i; j < information().cols(); ++j) {
        is >> information()(i, j);
        if(i != j) {
          information()(j, i) = information()(i, j);
        }
      }
    }
    return true;
  }

  virtual bool write(std::ostream& os) const override {
    Eigen::Vector2d v = _measurement;
    os << v(0) << " " << v(1) << " ";

    for(int i = 0; i < information().rows(); ++i) {
      for(int j = i; j < information().cols(); ++j) {
        os << " " << information()(i, j);
      };
    }
    return os.good();
  }

  virtual void setMeasurement(const Eigen::Vector2d& m) override {
    _measurement = m;
  }

};
class EdgeRoomXPlane : public BaseBinaryEdge<1, double, g2o::VertexRoomXYLB, g2o::VertexPlane> {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  EdgeRoomXPlane() : BaseBinaryEdge<1, double, g2o::VertexRoomXYLB, g2o::VertexPlane>() {
    _information.setZero();
    _error.setZero();
  }

  void computeError() override {
    const VertexRoomXYLB* v1 = static_cast<const VertexRoomXYLB*>(_vertices[0]);
    const VertexPlane* v2 = static_cast<const VertexPlane*>(_vertices[1]);
    Eigen::Vector2d t = v1->estimate();
    Eigen::Vector4d p = v2->estimate().coeffs();

    if(fabs(p(0)) > fabs(p(1)) && p(0) < 0) 
      p(3) = -1*p(3);
    else if(fabs(p(1)) > fabs(p(0)) && p(1) < 0) 
      p(3) = -1*p(3);

    double est; 
    if(fabs(t(0)) > fabs(p(3))) {
       est =  t(0) - p(3);
    } else {
       est =  p(3) - t(0);
    }

    _error[0] = est - _measurement;
  }

  virtual bool read(std::istream& is) override {
    double v;
    is >> v;

    setMeasurement(v);
    for(int i = 0; i < information().rows(); ++i) {
      for(int j = i; j < information().cols(); ++j) {
        is >> information()(i, j);
        if(i != j) {
          information()(j, i) = information()(i, j);
        }
      }
    }

    return true;
  }

  virtual bool write(std::ostream& os) const override {
    os << _measurement << " ";

    for(int i = 0; i < information().rows(); ++i) {
      for(int j = i; j < information().cols(); ++j) {
        os << " " << information()(i, j);
      };
    }
    return os.good();
  }

  virtual void setMeasurement(const double& m) override {
    _measurement = m;
  }

  virtual int measurementDimension() const override {
    return 1;
  }

};

class EdgeRoomYPlane : public BaseBinaryEdge<1, double, VertexRoomXYLB, g2o::VertexPlane> {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  EdgeRoomYPlane() : BaseBinaryEdge<1, double, VertexRoomXYLB, g2o::VertexPlane>() {}

  void computeError() override {
    const VertexRoomXYLB* v1 = static_cast<const VertexRoomXYLB*>(_vertices[0]);
    const VertexPlane* v2 = static_cast<const VertexPlane*>(_vertices[1]);
    Eigen::Vector2d t = v1->estimate();
    Eigen::Vector4d p = v2->estimate().coeffs();

    if(fabs(p(0)) > fabs(p(1)) && p(0) < 0) 
      p(3) = -1*p(3);
    else if(fabs(p(1)) > fabs(p(0)) && p(1) < 0) 
      p(3) = -1*p(3);

    double est; 
    if(fabs(t(1)) > fabs(p(3))) {
       est =  t(1) - p(3);
    } else {
       est =  p(3) - t(1);
    }

    _error[0] = est - _measurement;
  }

  virtual bool read(std::istream& is) override {
    double v;
    is >> v;

    setMeasurement(v);
    for(int i = 0; i < information().rows(); ++i) {
      for(int j = i; j < information().cols(); ++j) {
        is >> information()(i, j);
        if(i != j) {
          information()(j, i) = information()(i, j);
        }
      }
    }

    return true;
  }

  virtual bool write(std::ostream& os) const override {
    os << _measurement << " ";

    for(int i = 0; i < information().rows(); ++i) {
      for(int j = i; j < information().cols(); ++j) {
        os << " " << information()(i, j);
      };
    }
    return os.good();
  }

  virtual void setMeasurement(const double& m) override {
    _measurement = m;
  }

  virtual int measurementDimension() const override {
    return 1;
  }
};

}
#endif  // EDGE_ROOM_PLANE_HPP