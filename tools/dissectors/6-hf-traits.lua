-- create the traits protocol
p_hf_traits = Proto("hf-traits", "HF Traits Protocol")

-- traits packet fields
local f_avatar_id = ProtoField.guid("hf_traits.avatar_id", "Avatar ID")
local f_trait_type = ProtoField.uint8("hf_traits.trait_type", "Trait Type")

p_hf_traits.fields = {
  f_avatar_id, f_trait_type
}

function p_hf_traits.dissector(buf, pinfo, tree)
  pinfo.cols.protocol = p_hf_traits.name

  trait_subtree = tree:add(p_hf_traits, buf())

  local i = 0

  trait_subtree:add(f_avatar_id, buf(i, 16))
  i = i + 16

  trait_subtree:add_le(f_trait_type, buf(i, 1))
end
