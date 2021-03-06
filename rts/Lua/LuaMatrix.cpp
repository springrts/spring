#include "LuaMatrix.h"

#include "lib/sol2/sol.hpp"

#include "LuaMatrixImpl.h"
#include "LuaUtils.h"

bool LuaMatrix::PushEntries(lua_State* L)
{
	REGISTER_LUA_CFUNC(GetMatrix);

	sol::state_view lua(L);
	auto gl = sol::stack::get<sol::table>(L, -1);

	gl.new_usertype<LuaMatrixImpl>("LuaMatrixImpl",

		sol::constructors<LuaMatrixImpl()>(),

		"Zero", &LuaMatrixImpl::Zero,

		"Identity", &LuaMatrixImpl::Identity,

		"SetMatrixElements", &LuaMatrixImpl::SetMatrixElements,

		"DeepCopy", &LuaMatrixImpl::DeepCopy,

		"InvertAffine", &LuaMatrixImpl::InvertAffine,
		"Invert", &LuaMatrixImpl::Invert,

		"Transpose", &LuaMatrixImpl::Transpose,

		"Translate", &LuaMatrixImpl::Translate,
		"Scale", sol::overload(
			sol::resolve<void(const float, const float, const float)>(&LuaMatrixImpl::Scale),
			sol::resolve<void(const float)>(&LuaMatrixImpl::Scale)),

		"RotateRad", &LuaMatrixImpl::RotateRad,
		"RotateDeg", &LuaMatrixImpl::RotateDeg,

		"RotateRadX", &LuaMatrixImpl::RotateRadX,
		"RotateRadY", &LuaMatrixImpl::RotateRadY,
		"RotateRadZ", &LuaMatrixImpl::RotateRadZ,

		"RotateDegX", &LuaMatrixImpl::RotateDegX,
		"RotateDegY", &LuaMatrixImpl::RotateDegY,
		"RotateDegZ", &LuaMatrixImpl::RotateDegZ,

		"UnitMatrix", &LuaMatrixImpl::UnitMatrix,
		"UnitPieceMatrix", &LuaMatrixImpl::UnitPieceMatrix,

		"FeatureMatrix", &LuaMatrixImpl::FeatureMatrix,
		"FeaturePieceMatrix", &LuaMatrixImpl::FeaturePieceMatrix,

		"ProjectileMatrix", &LuaMatrixImpl::ProjectileMatrix,

		"ScreenViewMatrix", &LuaMatrixImpl::ScreenViewMatrix,
		"ScreenProjMatrix", &LuaMatrixImpl::ScreenProjMatrix,
		"ScreenViewProjMatrix", & LuaMatrixImpl::ScreenViewProjMatrix,

		"ShadowViewMatrix", &LuaMatrixImpl::ShadowViewMatrix,
		"ShadowProjMatrix", &LuaMatrixImpl::ShadowProjMatrix,
		"ShadowViewProjMatrix", & LuaMatrixImpl::ShadowViewProjMatrix,

		"Ortho", &LuaMatrixImpl::Ortho,
		"Frustum", &LuaMatrixImpl::Frustum,
		"LookAt", &LuaMatrixImpl::LookAt,

		"CameraViewMatrix", &LuaMatrixImpl::CameraViewMatrix,
		"CameraProjMatrix", &LuaMatrixImpl::CameraProjMatrix,
		"CameraViewProjMatrix", &LuaMatrixImpl::CameraViewProjMatrix,
		"CameraViewInverseMatrix", &LuaMatrixImpl::CameraViewInverseMatrix,
		"CameraProjInverseMatrix", &LuaMatrixImpl::CameraProjInverseMatrix,
		"CameraViewProjInverseMatrix", &LuaMatrixImpl::CameraViewProjInverseMatrix,
		"CameraBillboardMatrix", &LuaMatrixImpl::CameraBillboardMatrix,

		"GetAsScalar", &LuaMatrixImpl::GetAsScalar,
		"GetAsTable", &LuaMatrixImpl::GetAsTable,

		sol::meta_function::multiplication, sol::overload(
			sol::resolve< LuaMatrixImpl(const LuaMatrixImpl&) const >(&LuaMatrixImpl::operator*),
			sol::resolve< sol::as_table_t<float4Proxy>(const sol::table&) const >(&LuaMatrixImpl::operator*)
		),

		sol::meta_function::addition, &LuaMatrixImpl::operator+
		);

	gl.set("LuaMatrixImpl", sol::lua_nil); //because it's awkward :)

	return true;
}

int LuaMatrix::GetMatrix(lua_State* L)
{
	return sol::stack::call_lua(L, 1, [] { return LuaMatrixImpl{}; });
}