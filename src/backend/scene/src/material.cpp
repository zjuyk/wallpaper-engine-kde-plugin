#include <iostream>
#include "material.h"
#include "pkg.h"
#include "wallpaper.h"
#include "teximage.h"

using json = nlohmann::json;
namespace wp = wallpaper;

bool wp::Material::From_json(const json& obj_json) {
    if(!obj_json.contains("passes")) return false;
    auto& content = obj_json.at("passes")[0];
    if(!content.contains("shader")) return false;
    shader_ = content.at("shader");
	if(content.contains("textures"))
		for(auto& t:content.at("textures"))
			textures_.push_back(t.is_null()?"":t);
    if(!content.contains("depthtest"))
        depthtest_ = content.at("depthtest") == "disabled" ? false : true;
	if(content.contains("combos")) {
		for(auto& c:content.at("combos").items())
			combos_[c.key()] = c.value().get<int>();
		shader_.append("+");
		for(auto& c:combos_)
			shader_.append(c.first + std::to_string(c.second));
	}
	if(content.contains("constantshadervalues")) {
		constShadervalues_ = content.at("constantshadervalues").dump();
	}
    return true;
}

void wp::Material::Load(WPRender& wpRender) {
	LOG_INFO("compile " + shader_);
	wpRender.shaderMgr.CreateShader(shader_, combos_, shadervalues_);
	auto* lks = wpRender.shaderMgr.CreateLinkedShader(shader_);
	/*
	for(auto& el:lks->GetUniforms()) {
		LOG_INFO(el.name);
	}*/
	wpRender.shaderMgr.SetTextures(shader_, shadervalues_);
	// load constShadervalues as glname 
	if(!constShadervalues_.empty()) {
		auto constShadervalues_json = json::parse(constShadervalues_);
		for(auto& c:constShadervalues_json.items()) {
			std::string glname = gl::Shadervalue::FindShadervalue(shadervalues_, c.key());
			if(!glname.empty()) {
				auto& sv = shadervalues_[glname];
				if(c.value().is_string())
					gl::Shadervalue::SetValue(sv, c.value());
				else if(c.value().is_number())
					sv.value = std::vector<float>({c.value().get<float>()});
			}
		}
	}

	std::vector<gl::Shadervalue> sv_resolutions;
	for(auto&el:shadervalues_) {
		if(el.second.glname.compare(0, 9, "g_Texture") == 0 && el.second.glname.size()<11) {
			int index = std::stoi(&el.second.glname[9]);
			
			if(index >= textures_.size()) 
				textures_.resize(index+1, "_rt_replace");
			if(!el.second.value_str.empty() && textures_[index] == "_rt_replace") {
					textures_[index] = el.second.value_str;
			}
			auto& texture = textures_[index];

			//resolution and load tex
			std::string sv_resolution = "g_Texture"+std::to_string(index)+"Resolution";
			sv_resolutions.push_back(gl::Shadervalue());
			sv_resolutions.back().glname = sv_resolution;

			if(texture.empty() || texture.compare(0,4,"_rt_") == 0) {
				std::vector<int> re = {size_[0],size_[1],size_[0],size_[1]};
				sv_resolutions.back().value = std::vector<float>(re.begin(),re.end());
				continue;
			}
			std::cout << "load "+texture << std::endl;
			gl::Texture* tex = wpRender.texCache.LoadTexture(texture);
			auto resolution = tex->GetResolution();
			sv_resolutions.back().value = std::vector<float>(resolution.begin(), resolution.end());
		}
	}
	for(auto& el:sv_resolutions)
		shadervalues_[el.glname] = el;
}

void wp::Material::Render(WPRender& wpRender) {
	wpRender.shaderMgr.BindShader(shader_);
	for(int i=0;i < textures_.size();i++){
		if(textures_[i].empty() || textures_[i].compare(0,4,"_rt_") == 0) 
			continue;
		wpRender.glWrapper.ActiveTexture(i);
		auto* tex = wpRender.texCache.GetTexture(textures_[i]);
		tex->Bind();
	}
	wpRender.shaderMgr.BindShader(shader_);
	wpRender.shaderMgr.UpdateUniforms(shader_, shadervalues_);
	object_.CurVertices()->Draw();
	CHECK_GL_ERROR_IF_DEBUG();
}

void wp::Material::SetShadervalue(const std::string& glname, const std::vector<float>& value) {
	gl::Shadervalue sv = gl::Shadervalue();
	sv.glname = glname;
	sv.value = value;
	shadervalues_[glname] = sv;
}
