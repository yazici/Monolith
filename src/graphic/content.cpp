#include <cassert>
#include "content.hpp"
#include "core/texture.hpp"
#include "core/device.hpp"

namespace Graphic {

	Content::Content() :
		voxelRenderEffect( "shader/voxel.vs", "shader/voxel.gs", "shader/voxel.ps", Graphic::RasterizerState::CULL_MODE::BACK, Graphic::RasterizerState::FILL_MODE::SOLID ),
		texture2DEffect( "shader/screentex.vs", "shader/screentex.gs", "shader/screentex.ps", Graphic::RasterizerState::CULL_MODE::BACK, Graphic::RasterizerState::FILL_MODE::SOLID, Graphic::BlendState::BLEND_OPERATION::ADD, Graphic::BlendState::BLEND::SRC_ALPHA, Graphic::BlendState::BLEND::INV_SRC_ALPHA, Graphic::DepthStencilState::COMPARISON_FUNC::ALWAYS, false),
		wireEffect( "shader/wire.vs", "shader/wire.ps", Graphic::RasterizerState::CULL_MODE::BACK, Graphic::RasterizerState::FILL_MODE::SOLID, BlendState::BLEND_OPERATION::ADD, BlendState::BLEND::SRC_ALPHA, BlendState::BLEND::ONE, Graphic::DepthStencilState::COMPARISON_FUNC::LESS, false ),
		beamEffect( "shader/beam.vs", "shader/beam.gs", "shader/beam.ps", Graphic::RasterizerState::CULL_MODE::BACK, Graphic::RasterizerState::FILL_MODE::SOLID, BlendState::BLEND_OPERATION::ADD, BlendState::BLEND::SRC_ALPHA, BlendState::BLEND::ONE, Graphic::DepthStencilState::COMPARISON_FUNC::LESS, false ),
		voxelObjectUBO( "Object" ), wireObjectUBO( "Object" ), cameraUBO( "Camera" ), globalUBO( "Global" ),
		pointSampler(Graphic::SamplerState::EDGE_TREATMENT::WRAP, Graphic::SamplerState::SAMPLE::POINT,
					Graphic::SamplerState::SAMPLE::POINT, Graphic::SamplerState::SAMPLE::LINEAR ),
		linearSampler(Graphic::SamplerState::EDGE_TREATMENT::WRAP, Graphic::SamplerState::SAMPLE::LINEAR,
					Graphic::SamplerState::SAMPLE::LINEAR, Graphic::SamplerState::SAMPLE::LINEAR )
	{
		// Init the constant buffers
		voxelObjectUBO.AddAttribute( "WorldViewProjection", Graphic::UniformBuffer::ATTRIBUTE_TYPE::MATRIX );
		voxelObjectUBO.AddAttribute( "WorldView", Graphic::UniformBuffer::ATTRIBUTE_TYPE::MATRIX );
		voxelObjectUBO.AddAttribute( "Corner000", Graphic::UniformBuffer::ATTRIBUTE_TYPE::VEC4 );
		voxelObjectUBO.AddAttribute( "Corner001", Graphic::UniformBuffer::ATTRIBUTE_TYPE::VEC4 );
		voxelObjectUBO.AddAttribute( "Corner010", Graphic::UniformBuffer::ATTRIBUTE_TYPE::VEC4 );
		voxelObjectUBO.AddAttribute( "Corner011", Graphic::UniformBuffer::ATTRIBUTE_TYPE::VEC4 );
		voxelObjectUBO.AddAttribute( "Corner100", Graphic::UniformBuffer::ATTRIBUTE_TYPE::VEC4 );
		voxelObjectUBO.AddAttribute( "Corner101", Graphic::UniformBuffer::ATTRIBUTE_TYPE::VEC4 );
		voxelObjectUBO.AddAttribute( "Corner110", Graphic::UniformBuffer::ATTRIBUTE_TYPE::VEC4 );
		voxelObjectUBO.AddAttribute( "Corner111", Graphic::UniformBuffer::ATTRIBUTE_TYPE::VEC4 );

		wireObjectUBO.AddAttribute( "WorldViewProjection", Graphic::UniformBuffer::ATTRIBUTE_TYPE::MATRIX );
		wireObjectUBO.AddAttribute( "LineWidth", Graphic::UniformBuffer::ATTRIBUTE_TYPE::FLOAT );
		wireObjectUBO.AddAttribute( "BlendSlope", Graphic::UniformBuffer::ATTRIBUTE_TYPE::FLOAT );
		wireObjectUBO.AddAttribute( "BlendOffset", Graphic::UniformBuffer::ATTRIBUTE_TYPE::FLOAT );

		cameraUBO.AddAttribute( "View", Graphic::UniformBuffer::ATTRIBUTE_TYPE::MATRIX );
		cameraUBO.AddAttribute( "Projection", Graphic::UniformBuffer::ATTRIBUTE_TYPE::MATRIX );
		cameraUBO.AddAttribute( "ViewProjection", Graphic::UniformBuffer::ATTRIBUTE_TYPE::MATRIX );
		cameraUBO.AddAttribute( "ProjectionInverse", Graphic::UniformBuffer::ATTRIBUTE_TYPE::VEC4 );
		cameraUBO.AddAttribute( "Position", Graphic::UniformBuffer::ATTRIBUTE_TYPE::VEC3 );
		//cameraUBO.AddAttribute( "Far", Graphic::UniformBuffer::ATTRIBUTE_TYPE::FLOAT );

		globalUBO.AddAttribute( "Aspect", Graphic::UniformBuffer::ATTRIBUTE_TYPE::FLOAT );
		globalUBO.AddAttribute( "Width", Graphic::UniformBuffer::ATTRIBUTE_TYPE::FLOAT );
		globalUBO.AddAttribute( "Height", Graphic::UniformBuffer::ATTRIBUTE_TYPE::FLOAT );
		globalUBO.AddAttribute( "Time", Graphic::UniformBuffer::ATTRIBUTE_TYPE::FLOAT );
		Math::IVec2 size = Graphic::Device::GetFramebufferSize();
		globalUBO["Aspect"] = Graphic::Device::GetAspectRatio();
		globalUBO["Width"] = (float)size[0];
		globalUBO["Height"] = (float)size[1];

		// Bind constant buffers to effects
		voxelRenderEffect.BindUniformBuffer( voxelObjectUBO );
		voxelRenderEffect.BindUniformBuffer( cameraUBO );
		voxelRenderEffect.BindUniformBuffer( globalUBO );
		voxelRenderEffect.BindTexture("u_diffuseTex", 0, pointSampler);

		wireEffect.BindUniformBuffer( wireObjectUBO );
		beamEffect.BindUniformBuffer( wireObjectUBO );

		//texture2DEffect.BindUniformBuffer( globalUBO );

		defaultFont = new Font("arial", this);
		gameFont = new Font("MonoLith", this);
	}


	Content::~Content()
	{
		delete defaultFont;
		delete gameFont;
	}


} // namespace Graphic