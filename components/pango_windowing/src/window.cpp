#include <pangolin/factory/RegisterFactoriesWindowInterface.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/windowing/window.h>

namespace pangolin
{

Shared<WindowInterface> WindowInterface::Create(
    const WindowInterface::Params& params)
{
  RegisterFactoriesWindowInterface();
  return FactoryRegistry::I()->Construct<WindowInterface>(params.uri);
}

}  // namespace pangolin