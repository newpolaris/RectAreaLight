#pragma once

#include <GLType/GraphicsDevice.h>

class OGLDevice final : public GraphicsDevice
{
    __DeclareSubInterface(OGLDevice, GraphicsDevice)
public:

    OGLDevice() noexcept;
    virtual ~OGLDevice() noexcept;

    bool create(const GraphicsDeviceDesc& desc) noexcept;
    void destoy() noexcept;

    GraphicsDataPtr createGraphicsData(const GraphicsDataDesc& desc) noexcept;

	const GraphicsDeviceDesc& getGraphicsDeviceDesc() const noexcept;

private:

    GraphicsDeviceDesc m_Desc;
};
