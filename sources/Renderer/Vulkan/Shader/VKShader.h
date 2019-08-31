/*
 * VKShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_SHADER_H
#define LLGL_VK_SHADER_H


#include <LLGL/Shader.h>
#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


struct ShaderReflection;
struct Extent3D;

class VKShader final : public Shader
{

    public:

        bool HasErrors() const override;

        std::string GetReport() const override;

    public:

        VKShader(const VKPtr<VkDevice>& device, const ShaderDescriptor& desc);

        void FillShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo) const;

        bool Reflect(ShaderReflection& reflection) const;
        bool ReflectLocalSize(Extent3D& localSize) const;

        // Returns the Vulkan shader module.
        inline const VKPtr<VkShaderModule>& GetShaderModule() const
        {
            return shaderModule_;
        }

    private:

        // Note: "Success" is a reserved macro by X11 lib.
        enum class LoadBinaryResult
        {
            Undefined,
            Successful,
            InvalidCodeSize,
            ReflectFailed,
        };

    private:

        bool Build(const ShaderDescriptor& shaderDesc);
        bool CompileSource(const ShaderDescriptor& shaderDesc);
        bool LoadBinary(const ShaderDescriptor& shaderDesc);

    private:

        VkDevice                device_             = VK_NULL_HANDLE;
        VKPtr<VkShaderModule>   shaderModule_;
        std::vector<char>       shaderModuleData_;
        LoadBinaryResult        loadBinaryResult_   = LoadBinaryResult::Undefined;

        std::string             entryPoint_;
        std::string             errorLog_;

};


} // /namespace LLGL


#endif



// ================================================================================
