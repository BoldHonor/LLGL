/*
 * D3D11ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ShaderProgram.h"
#include "D3D11Shader.h"
#include "../D3D11Types.h"
#include "../../CheckedCast.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Helper.h"
#include <LLGL/Log.h>
#include <LLGL/VertexFormat.h>
#include <algorithm>
#include <stdexcept>


namespace LLGL
{


D3D11ShaderProgram::D3D11ShaderProgram(ID3D11Device* device) :
    device_ { device }
{
}

void D3D11ShaderProgram::AttachShader(Shader& shader)
{
    /* Store D3D11 shader */
    auto shaderD3D = LLGL_CAST(D3D11Shader*, &shader);

    const auto shaderType0Idx = static_cast<std::size_t>(ShaderType::Vertex);
    const auto shaderTypeNIdx = (static_cast<std::size_t>(shader.GetType()) - shaderType0Idx);

    if (shaderTypeNIdx < 6)
        shaders_[shaderTypeNIdx] = shaderD3D;
    else
        throw std::runtime_error("cannot attach shader with invalid type: 0x" + ToHex(shaderTypeNIdx));
}

void D3D11ShaderProgram::DetachAll()
{
    /* Reset all shader attributes */
    inputLayout_.Reset();
    std::fill(std::begin(shaders_), std::end(shaders_), nullptr);
    linkError_ = LinkError::NoError;
}

bool D3D11ShaderProgram::LinkShaders()
{
    /* Validate shader composition */
    linkError_ = LinkError::NoError;

    /* Validate native shader objects */
    for (std::size_t i = 0; i < 6; ++i)
    {
        if (shaders_[i] != nullptr && shaders_[i]->GetNative().vs == nullptr)
            linkError_ = LinkError::InvalidByteCode;
    }

    /*
    Validate composition of attached shaders
    Note: reinterpret_cast allowed here, because no multiple inheritance is used, just plain pointers!
    */
    if (!ValidateShaderComposition(reinterpret_cast<Shader* const*>(shaders_), 6))
        linkError_ = LinkError::InvalidComposition;

    return (linkError_ == LinkError::NoError);
}

std::string D3D11ShaderProgram::QueryInfoLog()
{
    if (auto s = ShaderProgram::LinkErrorToString(linkError_))
        return s;
    else
        return "";
}

ShaderReflectionDescriptor D3D11ShaderProgram::QueryReflectionDesc() const
{
    ShaderReflectionDescriptor reflection;

    /* Reflect all shaders */
    for (auto shader : shaders_)
    {
        if (shader != nullptr)
            shader->Reflect(reflection);
    }

    /* Sort output to meet the interface requirements */
    ShaderProgram::FinalizeShaderReflection(reflection);

    return reflection;
}

static DXGI_FORMAT GetInputElementFormat(const VertexAttribute& attrib)
{
    try
    {
        return D3D11Types::Map(attrib.vectorType);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument(std::string(e.what()) + " (for vertex attribute \"" + attrib.name + "\")");
    }
}

void D3D11ShaderProgram::BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats)
{
    if (numVertexFormats == 0 || vertexFormats == nullptr)
        return;

    if (!vs_ || vs_->GetByteCode().empty())
        throw std::runtime_error("cannot build input layout without valid vertex shader");

    /* Setup input element descriptors */
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;

    for (std::uint32_t i = 0; i < numVertexFormats; ++i)
    {
        for (const auto& attrib : vertexFormats[i].attributes)
        {
            D3D11_INPUT_ELEMENT_DESC elementDesc;
            {
                elementDesc.SemanticName            = attrib.name.c_str();
                elementDesc.SemanticIndex           = attrib.semanticIndex;
                elementDesc.Format                  = GetInputElementFormat(attrib);
                elementDesc.InputSlot               = vertexFormats[i].inputSlot;
                elementDesc.AlignedByteOffset       = attrib.offset;
                elementDesc.InputSlotClass          = (attrib.instanceDivisor > 0 ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA);
                elementDesc.InstanceDataStepRate    = attrib.instanceDivisor;
            }
            inputElements.push_back(elementDesc);
        }
    }

    /* Create input layout */
    inputLayout_.Reset();
    auto hr = device_->CreateInputLayout(
        inputElements.data(),
        static_cast<UINT>(inputElements.size()),
        vs_->GetByteCode().data(),
        vs_->GetByteCode().size(),
        &inputLayout_
    );
    DXThrowIfFailed(hr, "failed to create D3D11 input layout");
}

void D3D11ShaderProgram::BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    // dummy
}

void D3D11ShaderProgram::BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    // dummy
}

ShaderUniform* D3D11ShaderProgram::LockShaderUniform()
{
    return nullptr; // dummy
}

void D3D11ShaderProgram::UnlockShaderUniform()
{
    // dummy
}


} // /namespace LLGL



// ================================================================================
