#include "effect.h"
#include "pkg.h"
#include "wallpaper.h"

using json = nlohmann::json;
using namespace wallpaper;

bool Effect::From_json(const nlohmann::json& effect_j) {
	auto& pkgfs = WallpaperGL::GetPkgfs();
	// effect code is at "file"
	std::string file_src = fs::GetContent(pkgfs, effect_j.at("file"));
	auto file_j = json::parse(file_src); 
	name_ = file_j.at("name");	
	// load fbos
	if(file_j.contains("fbos")) {
		auto& fbos = file_j.at("fbos");
		for(auto& f:fbos)
			fboMap_[f.at("name").get<std::string>()] = nullptr;
	}
	fboMap_["default"] = nullptr;
	// effect code must have passes, which contain material
	if(!file_j.contains("passes")) return false;	
	auto& file_passes_j = file_j.at("passes");
	// pass effect passes to effect code
	if(effect_j.contains("passes")) {
		auto& passes_j = effect_j.at("passes");
		auto it_file=file_passes_j.begin();
		for(auto& el:passes_j)
			(*it_file++)["material_pass"] = el;
	}
	int index = 0;
	for(auto& pass_j:file_passes_j) {
		//load material
		if(!pass_j.contains("material")) return false;
		std::string material_src = fs::GetContent(pkgfs, pass_j.at("material"));
		auto material_j = json::parse(material_src);
		auto& content_j = material_j.at("passes")[0];
		// pass effect passes to material
		if(pass_j.contains("material_pass")) {
			auto& material_pass_j = pass_j.at("material_pass");
			for(auto& el:material_pass_j.items())
				content_j[el.key()] = el.value();
		}
		materials_.emplace_back((RenderObject&)imgObject_, size_);
		auto& material = materials_.back();
		material.material.From_json(material_j);
		//load bind
		if(pass_j.contains("bind")) {
			auto& binds_j = pass_j.at("bind");
			for(auto& b:binds_j) {
				BindInfo bi;
				bi.name = b.at("name");
				bi.index = b.at("index");
				material.bindInfos.push_back(bi);
			}
		}
		else material.bindInfos.push_back({"previous",0});
		material.target = pass_j.contains("target")?pass_j.at("target"):"default";
		index++;
	}
	return true;
}

void Effect::Load(WPRender& wpRender) {
	auto modelviewpro_mat = glm::mat4(1.0f);
	const float* modelviewpro  = glm::value_ptr(modelviewpro_mat);

	for(auto& f:fboMap_)
		f.second = std::unique_ptr<gl::GLFramebuffer>(wpRender.glWrapper.CreateFramebuffer(size_[0], size_[1]));
	for(auto& m:materials_) {
		m.material.SetShadervalue("g_ModelViewProjectionMatrix", std::vector<float>(modelviewpro, modelviewpro + 4*4));
		m.material.Load(wpRender);
	}
	auto size = wpRender.shaderMgr.globalUniforms.Ortho();
	vertices_ = gl::VerticeArray::GenDefault(&wpRender.glWrapper);
	vertices_.Update();
}
		
void Effect::Render(WPRender& wpRender) {
	imgObject_.SetCurVertices(&vertices_);
	for(auto& m:materials_) {
		wpRender.glWrapper.BindFramebuffer(fboMap_.at(m.target).get());
		glViewport(0,0,size_[0], size_[1]);
		wpRender.Clear(0.0f);

		for(auto& b:m.bindInfos) {
			wpRender.glWrapper.ActiveTexture(b.index);
			if(b.name == "previous")
				wpRender.glWrapper.BindTexture(&(imgObject_.CurFbo())->color_texture);
			else
				wpRender.glWrapper.BindTexture(&fboMap_.at(b.name)->color_texture);
		}
	    m.material.Render(wpRender);
	}
	imgObject_.SetCurFbo(fboMap_.at(materials_.back().target).get());
}
