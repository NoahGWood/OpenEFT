#include "glarea.h"
#include "glarea_setting.h"

void GLAreaSetting::initGlobalParameterSet( RichParameterSet * defaultGlobalParamSet)
{
    defaultGlobalParamSet->addParam(new RichColor(backgroundBotColorParam(),QColor( 54, 43, 183),"MeshLab Bottom BackGround Color ","MeshLab GLarea's BackGround Color(bottom corner)"));
    defaultGlobalParamSet->addParam(new RichColor(backgroundTopColorParam(),QColor( 24,102,219),"MeshLab Top BackGround Color","MeshLab GLarea's BackGround Color(top corner)"));
    defaultGlobalParamSet->addParam(new RichColor(logAreaColorParam(),   QColor( 54, 43,183),"MeshLab GLarea's Log Area Color","MeshLab GLarea's BackGround Color(bottom corner)"));


    defaultGlobalParamSet->addParam(new RichColor(baseLightAmbientColorParam()	,QColor( 32, 32, 32),"MeshLab Base Light Ambient Color","MeshLab GLarea's BackGround Color(bottom corner)"));
    defaultGlobalParamSet->addParam(new RichColor(baseLightDiffuseColorParam()	,QColor(204,204,204),"MeshLab Base Light Diffuse Color","MeshLab GLarea's BackGround Color(top corner)"));
    defaultGlobalParamSet->addParam(new RichColor(baseLightSpecularColorParam() ,QColor(255,255,255),"MeshLab Base Light Specular Color","MeshLab GLarea's BackGround Color(bottom corner)"));

    defaultGlobalParamSet->addParam(new RichColor(fancyBLightDiffuseColorParam()	,QColor(255,204,204),"MeshLab Base Light Diffuse Color","MeshLab GLarea's BackGround Color(top corner)"));
    defaultGlobalParamSet->addParam(new RichColor(fancyFLightDiffuseColorParam()	,QColor(204,204,255),"MeshLab Base Light Diffuse Color","MeshLab GLarea's BackGround Color(top corner)"));

    QStringList textureMinFilterModes =  (QStringList() << "Nearest" << "MipMap");
    QStringList textureMagFilterModes =  (QStringList() << "Nearest" << "Linear");
    defaultGlobalParamSet->addParam(new RichEnum(textureMinFilterParam()	, 1,textureMinFilterModes,"MeshLab Texture Minification Filtering","MeshLab GLarea's BackGround Color(top corner)"));
    defaultGlobalParamSet->addParam(new RichEnum(textureMagFilterParam()	, 1,textureMagFilterModes,"MeshLab Texture Magnification Filtering","MeshLab GLarea's BackGround Color(top corner)"));
}


void GLAreaSetting::updateGlobalParameterSet( RichParameterSet& rps )
{
    logAreaColor = rps.getColor4b(logAreaColorParam());
    backgroundBotColor =  rps.getColor4b(backgroundBotColorParam());
    backgroundTopColor =  rps.getColor4b(backgroundTopColorParam());

    baseLightAmbientColor =  rps.getColor4b(baseLightAmbientColorParam()	);
    baseLightDiffuseColor =  rps.getColor4b(baseLightDiffuseColorParam()	);
    baseLightSpecularColor =  rps.getColor4b(baseLightSpecularColorParam() );

    fancyBLightDiffuseColor =  rps.getColor4b(fancyBLightDiffuseColorParam());
    fancyFLightDiffuseColor =  rps.getColor4b(fancyFLightDiffuseColorParam());

    textureMinFilter = rps.getEnum(this->textureMinFilterParam());
    textureMagFilter = rps.getEnum(this->textureMagFilterParam());

}
