/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "wrappers/pipeline.hpp"

namespace throttle::graphics {

vk::raii::DescriptorSetLayout descriptor_set_data::create_decriptor_set_layout(
    const vk::raii::Device                                                            &p_device,
    const std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> &p_binding_data) {
  std::vector<vk::DescriptorSetLayoutBinding> bindings;
  bindings.reserve(p_binding_data.size());
  std::transform(p_binding_data.begin(), p_binding_data.end(), std::back_inserter(bindings),
                 [i = uint32_t{0}](auto &elem) mutable {
                   return vk::DescriptorSetLayoutBinding{i++, std::get<0>(elem), std::get<1>(elem), std::get<2>(elem)};
                 });
  vk::DescriptorSetLayoutCreateInfo descriptor_set_info = {.bindingCount = static_cast<uint32_t>(bindings.size()),
                                                           .pBindings = bindings.data()};
  return vk::raii::DescriptorSetLayout{p_device, descriptor_set_info};
}

vk::raii::DescriptorPool
descriptor_set_data::create_descriptor_pool(const vk::raii::Device                    &p_device,
                                            const std::vector<vk::DescriptorPoolSize> &p_pool_sizes) {
  uint32_t max_sets =
      std::accumulate(p_pool_sizes.begin(), p_pool_sizes.end(), 0,
                      [](uint32_t sum, vk::DescriptorPoolSize const &dps) { return sum + dps.descriptorCount; });
  vk::DescriptorPoolCreateInfo descriptor_info = {.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                                  .maxSets = max_sets,
                                                  .poolSizeCount = static_cast<uint32_t>(p_pool_sizes.size()),
                                                  .pPoolSizes = p_pool_sizes.data()};
  return vk::raii::DescriptorPool(p_device, descriptor_info);
}

} // namespace throttle::graphics