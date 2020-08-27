#include "UniformConstants.h"

#include <cassert>

#include "Rendering/GlobalRendering.h"
#include "Rendering/ShadowHandler.h"
#include "Game/Camera.h"
#include "Game/CameraHandler.h"
#include "Game/GlobalUnsynced.h"
#include "Sim/Misc/GlobalConstants.h"
#include "Sim/Misc/GlobalSynced.h"
#include "Map/ReadMap.h"
#include "System/Log/ILog.h"
#include "System/SafeUtil.h"



void UniformConstants::InitVBO(std::unique_ptr<VBO>& vbo, const int vboSingleSize)
{
	vbo = std::make_unique<VBO>(GL_UNIFORM_BUFFER, PERSISTENT_STORAGE);
	vbo->Bind(GL_UNIFORM_BUFFER);
	vbo->New(BUFFERING * vboSingleSize, GL_DYNAMIC_DRAW); //allocate BUFFERING times the buffer size for non-blocking updates
	vbo->Unbind();
}


void UniformConstants::Init()
{
	if (!Supported()) {
		LOG_L(L_ERROR, "[UniformConstants::%s] Important OpenGL extensions are not supported by the system\n  GLEW_ARB_uniform_buffer_object = %d\n  GLEW_ARB_shading_language_420pack = %d", __func__, GLEW_ARB_uniform_buffer_object, GLEW_ARB_shading_language_420pack);
		return;
	}

	{
		GLint uniformBufferOffset = 0;

		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBufferOffset);

		umbBufferSize = ((sizeof(UniformMatricesBuffer) / uniformBufferOffset) + 1) * uniformBufferOffset;
		upbBufferSize = ((sizeof(UniformParamsBuffer) / uniformBufferOffset) + 1) * uniformBufferOffset;
	}

	InitVBO(umbBuffer, umbBufferSize);
	InitVBO(upbBuffer, upbBufferSize);
}

void UniformConstants::Kill()
{
	if (!Supported())
		return;

	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_MATRIX_IDX, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_PARAMS_IDX, 0);
}

void UniformConstants::UpdateMatrices(UniformMatricesBuffer* updateBuffer)
{
	updateBuffer->screenView = *globalRendering->screenViewMatrix;
	updateBuffer->screenProj = *globalRendering->screenProjMatrix;
	updateBuffer->screenViewProj = updateBuffer->screenProj * updateBuffer->screenView;

	const auto camPlayer = CCameraHandler::GetCamera(CCamera::CAMTYPE_PLAYER);

	updateBuffer->cameraView = camPlayer->GetViewMatrix();
	updateBuffer->cameraProj = camPlayer->GetProjectionMatrix();
	updateBuffer->cameraViewProj = camPlayer->GetViewProjectionMatrix();
	updateBuffer->cameraBillboard = updateBuffer->cameraView * camPlayer->GetBillBoardMatrix(); //GetBillBoardMatrix() is supposed to be multiplied by the view Matrix

	updateBuffer->cameraViewInv = camPlayer->GetViewMatrixInverse();
	updateBuffer->cameraProjInv = camPlayer->GetProjectionMatrixInverse();
	updateBuffer->cameraViewProjInv = camPlayer->GetViewProjectionMatrixInverse();

	updateBuffer->shadowView = shadowHandler.GetShadowViewMatrix(CShadowHandler::SHADOWMAT_TYPE_DRAWING);
	updateBuffer->shadowProj = shadowHandler.GetShadowProjMatrix(CShadowHandler::SHADOWMAT_TYPE_DRAWING);
	updateBuffer->shadowViewProj = updateBuffer->shadowProj * updateBuffer->shadowView;
}


void UniformConstants::UpdateParams(UniformParamsBuffer* updateBuffer)
{
	updateBuffer->timeInfo = float4{(float)gs->frameNum, gs->frameNum / (1.0f * GAME_SPEED), (float)globalRendering->drawFrame, globalRendering->timeOffset}; //gameFrame, gameSeconds, drawFrame, frameTimeOffset
	updateBuffer->viewGeometry = float4{(float)globalRendering->viewSizeX, (float)globalRendering->viewSizeY, (float)globalRendering->viewPosX, (float)globalRendering->viewPosY}; //vsx, vsy, vpx, vpy
	updateBuffer->mapSize = float4{(float)mapDims.mapx, (float)mapDims.mapy, (float)mapDims.pwr2mapx, (float)mapDims.pwr2mapy} *(float)SQUARE_SIZE; //xz, xzPO2

	updateBuffer->rndVec3 = float4{guRNG.NextVector(), 0.0f};
}


template<typename TBuffType, typename TUpdateFunc>
void UniformConstants::UpdateMap(std::unique_ptr<VBO>& vbo, TBuffType*& buffMap, const TUpdateFunc& updateFunc, const int vboSingleSize)
{
	TBuffType* thisFrameBuffMap = nullptr;

	unsigned int buffCurIdx = globalRendering->drawFrame % BUFFERING;
	vbo->Bind(GL_UNIFORM_BUFFER);

	if constexpr (PERSISTENT_STORAGE) {
		if (buffMap == nullptr) {
			buffMap = reinterpret_cast<TBuffType*>(vbo->MapBuffer(0, BUFFERING * vboSingleSize)); //map first time and forever
			assert(buffMap != nullptr);
		}

		thisFrameBuffMap = reinterpret_cast<TBuffType*> (reinterpret_cast<GLubyte*>(buffMap) + buffCurIdx * vboSingleSize); //choose the current part of the buffer
	} else {
		buffMap = reinterpret_cast<TBuffType*>(vbo->MapBuffer(buffCurIdx * vboSingleSize, vboSingleSize));
		assert(buffMap != nullptr);

		thisFrameBuffMap = buffMap;
	}

	updateFunc(thisFrameBuffMap);

	if constexpr(!PERSISTENT_STORAGE)
		vbo->UnmapBuffer();

	vbo->Unbind();
}

void UniformConstants::Update()
{
	if (!Supported())
		return;

	UpdateMap(umbBuffer, umbBufferMap, UniformConstants::UpdateMatrices, umbBufferSize);
	UpdateMap(upbBuffer, upbBufferMap, UniformConstants::UpdateParams  , upbBufferSize);
}

//lazy / implicit unbinding included
void UniformConstants::Bind()
{
	if (!Supported())
		return;

	assert(umbBuffer != nullptr && upbBuffer != nullptr);
	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_MATRIX_IDX, umbBuffer->GetId());
	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_PARAMS_IDX, upbBuffer->GetId());
}
