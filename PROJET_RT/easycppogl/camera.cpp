/*******************************************************************************
* EasyCppOGL:   Copyright (C) 2019,                                            *
* Sylvain Thery, IGG Group, ICube, University of Strasbourg, France            *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Contact information: thery@unistra.fr                                        *
*******************************************************************************/


#include <camera.h>

namespace EZCOGL
{

GLMat4 Camera::perspective(double znear, double zfar) const
{
	double range_inv = 1.0 / (znear - zfar);
	double f = 1.0/std::tan(field_of_view_/2.0);
	auto m05 = (asp_ratio_>1) ? std::make_pair(f/asp_ratio_,f) : std::make_pair(f,f*asp_ratio_);
	GLMat4 m;
	m << float(m05.first),  0,  0,  0,
		  0, float(m05.second),  0,  0,
		  0,  0, float((znear+zfar)*range_inv), float(2*znear*zfar*range_inv),
		  0,  0, -1 ,0;
	return m;
}

GLMat4 Camera::ortho(double znear, double zfar) const
{
	double range_inv = 1.0 / (znear - zfar);
	auto m05 = (asp_ratio_<1) ? std::make_pair(1.0/asp_ratio_,1.0) : std::make_pair(1.0,1.0/asp_ratio_);
	GLMat4 m;
	m << float(m05.first),  0,  0,  0,
		  0, float(m05.second),  0,  0,
		  0,  0, float(2*range_inv), 0,
		  0,  0, float((znear+zfar)*range_inv),0;
	return m;
}

Eigen::Matrix4d Camera::perspective_d(double znear, double zfar) const
{
	double range_inv = 1.0 / (znear - zfar);
	double f = 1.0/std::tan(field_of_view_/2.0);
	auto m05 = (asp_ratio_>1) ? std::make_pair(f/asp_ratio_,f) : std::make_pair(f,f*asp_ratio_);
	Eigen::Matrix4d m;
	m << m05.first,  0,  0,  0,
		  0, m05.second,  0,  0,
		  0,  0, (znear+zfar)*range_inv, 2*znear*zfar*range_inv,
		  0,  0, -1 ,0;
	return m;
}

Eigen::Matrix4d Camera::ortho_d(double znear, double zfar) const
{
	double range_inv = 1.0 / (znear - zfar);
	auto m05 = (asp_ratio_<1) ? std::make_pair(1.0/asp_ratio_,1.0) : std::make_pair(1.0,1.0/asp_ratio_);
	Eigen::Matrix4d m;
	m << m05.first,  0,  0,  0,
		  0, m05.second,  0,  0,
		  0,  0, 2*range_inv, 0,
		  0,  0, (znear+zfar)*range_inv,0;
	return m;
}


Eigen::Matrix4d Camera::get_projection_matrix_d() const
{
	double d = focal_dist_ - this->frame_.translation().z();
	znear_ = std::max(0.01, d - scene_radius_);
	zfar_ = d + scene_radius_;

	projd_ = ((type_==PERSPECTIVE) ? perspective_d(znear_,zfar_) : ortho_d(znear_,zfar_));
	return projd_;
}

Eigen::Matrix4d Camera::get_modelview_matrix_d() const
{
	Eigen::Affine3d aff = Eigen::Translation3d(Eigen::Vector3d(0.0,0.0,-focal_dist_)) * this->frame_ * Eigen::Translation3d(-pivot_point_);
	Eigen::Matrix4d md = aff.matrix();
	mvd_ = md;
	return mvd_;
}


}
