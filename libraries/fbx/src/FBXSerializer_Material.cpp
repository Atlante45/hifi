//
//  FBXSerializer_Material.cpp
//  interface/src/fbx
//
//  Created by Sam Gateau on 8/27/2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "FBXSerializer.h"

#include <iostream>
#include <memory>

#include <QBuffer>
#include <QDataStream>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QTextStream>
#include <QtDebug>
#include <QtEndian>

#include <hfm/ModelFormatLogging.h>

HFMTexture FBXSerializer::getTexture(const QString& textureID, const QString& materialID) {
    HFMTexture texture;
    const QByteArray& filepath = _textureFilepaths.value(textureID);
    texture.content = _textureContent.value(filepath);

    if (texture.content.isEmpty()) { // the content is not inlined
        texture.filename = _textureFilenames.value(textureID);
    } else { // use supplied filepath for inlined content
        texture.filename = filepath;
    }

    texture.id = textureID;
    texture.name = _textureNames.value(textureID);
    texture.transform.setIdentity();
    texture.texcoordSet = 0;
    if (_textureParams.contains(textureID)) {
        auto p = _textureParams.value(textureID);

        texture.transform.postTranslate(p.translation);
        texture.transform.postRotate(glm::quat(glm::radians(p.rotation)));

        auto scaling = p.scaling;
        // Protect from bad scaling which should never happen
        if (scaling.x == 0.0f) {
            scaling.x = 1.0f;
        }
        if (scaling.y == 0.0f) {
            scaling.y = 1.0f;
        }
        if (scaling.z == 0.0f) {
            scaling.z = 1.0f;
        }
        texture.transform.postScale(scaling);

        if ((p.UVSet != "map1") && (p.UVSet != "UVSet0")) {
            texture.texcoordSet = 1;
        }
        texture.texcoordSetName = p.UVSet;
    }
    auto materialParamItr = _materialParams.find(materialID);
    if (materialParamItr != _materialParams.end()) {
        auto& materialParam = materialParamItr.value();
        texture.transform.postTranslate(materialParam.translation);
        texture.transform.postScale(materialParam.scaling);
    }
    return texture;
}

void FBXSerializer::consolidateHFMMaterials() {
    for (QHash<QString, HFMMaterial>::iterator it = _hfmMaterials.begin(); it != _hfmMaterials.end(); it++) {
        HFMMaterial& material = (*it);

        // Maya is the exporting the shading model and we are trying to use it
        bool isMaterialLambert = (material.shadingModel.toLower() == "lambert");

        // the pure material associated with this part
        bool detectDifferentUVs = false;
        HFMTexture diffuseTexture;
        HFMTexture diffuseFactorTexture;
        QString diffuseTextureID = diffuseTextures.value(material.materialID);
        QString diffuseFactorTextureID = diffuseFactorTextures.value(material.materialID);

        // If both factor and color textures are specified, the texture bound to DiffuseColor wins
        if (!diffuseFactorTextureID.isNull() || !diffuseTextureID.isNull()) {
            if (!diffuseFactorTextureID.isNull() && diffuseTextureID.isNull()) {
                diffuseTextureID = diffuseFactorTextureID;
                // If the diffuseTextureID comes from the Texture bound to DiffuseFactor, we know it s exported from maya
                // And the DiffuseFactor is forced to 0.5 by Maya which is bad
                // So we need to force it to 1.0
                material.diffuseFactor = 1.0;
            }

            diffuseTexture = getTexture(diffuseTextureID, material.materialID);

            // FBX files generated by 3DSMax have an intermediate texture parent, apparently
            foreach (const QString& childTextureID, _connectionChildMap.values(diffuseTextureID)) {
                if (_textureFilenames.contains(childTextureID)) {
                    diffuseTexture = getTexture(diffuseTextureID, material.materialID);
                }
            }

            material.albedoTexture = diffuseTexture;
            detectDifferentUVs = (diffuseTexture.texcoordSet != 0) || (!diffuseTexture.transform.isIdentity());
        }

        HFMTexture transparentTexture;
        QString transparentTextureID = transparentTextures.value(material.materialID);
        // If PBS Material, systematically bind the albedo texture as transparency texture and check for the alpha channel
        if (material.isPBSMaterial) {
            transparentTextureID = diffuseTextureID;
        }
        if (!transparentTextureID.isNull()) {
            transparentTexture = getTexture(transparentTextureID, material.materialID);
            material.opacityTexture = transparentTexture;
            detectDifferentUVs |= (transparentTexture.texcoordSet != 0) || (!transparentTexture.transform.isIdentity());
        }

        HFMTexture normalTexture;
        QString bumpTextureID = bumpTextures.value(material.materialID);
        QString normalTextureID = normalTextures.value(material.materialID);
        if (!normalTextureID.isNull()) {
            normalTexture = getTexture(normalTextureID, material.materialID);
            normalTexture.isBumpmap = false;

            material.normalTexture = normalTexture;
            detectDifferentUVs |= (normalTexture.texcoordSet != 0) || (!normalTexture.transform.isIdentity());
        } else if (!bumpTextureID.isNull()) {
            normalTexture = getTexture(bumpTextureID, material.materialID);
            normalTexture.isBumpmap = true;

            material.normalTexture = normalTexture;
            detectDifferentUVs |= (normalTexture.texcoordSet != 0) || (!normalTexture.transform.isIdentity());
        }

        HFMTexture specularTexture;
        QString specularTextureID = specularTextures.value(material.materialID);
        if (!specularTextureID.isNull()) {
            specularTexture = getTexture(specularTextureID, material.materialID);
            detectDifferentUVs |= (specularTexture.texcoordSet != 0) || (!specularTexture.transform.isIdentity());
            material.specularTexture = specularTexture;
        }

        HFMTexture metallicTexture;
        QString metallicTextureID = metallicTextures.value(material.materialID);
        if (!metallicTextureID.isNull()) {
            metallicTexture = getTexture(metallicTextureID, material.materialID);
            detectDifferentUVs |= (metallicTexture.texcoordSet != 0) || (!metallicTexture.transform.isIdentity());
            material.metallicTexture = metallicTexture;
        }

        HFMTexture roughnessTexture;
        QString roughnessTextureID = roughnessTextures.value(material.materialID);
        if (!roughnessTextureID.isNull()) {
            roughnessTexture = getTexture(roughnessTextureID, material.materialID);
            material.roughnessTexture = roughnessTexture;
            detectDifferentUVs |= (roughnessTexture.texcoordSet != 0) || (!roughnessTexture.transform.isIdentity());
        }

        HFMTexture shininessTexture;
        QString shininessTextureID = shininessTextures.value(material.materialID);
        if (!shininessTextureID.isNull()) {
            shininessTexture = getTexture(shininessTextureID, material.materialID);
            material.glossTexture = shininessTexture;
            detectDifferentUVs |= (shininessTexture.texcoordSet != 0) || (!shininessTexture.transform.isIdentity());
        }

        HFMTexture emissiveTexture;
        QString emissiveTextureID = emissiveTextures.value(material.materialID);
        if (!emissiveTextureID.isNull()) {
            emissiveTexture = getTexture(emissiveTextureID, material.materialID);
            detectDifferentUVs |= (emissiveTexture.texcoordSet != 0) || (!emissiveTexture.transform.isIdentity());
            material.emissiveTexture = emissiveTexture;

            if (isMaterialLambert) {
                // If the emissiveTextureID comes from the Texture bound to Emissive when material is lambert, we know it s
                // exported from maya And the EMissiveColor is forced to 0.5 by Maya which is bad So we need to force it to 1.0
                material.emissiveColor = vec3(1.0);
            }
        }

        HFMTexture occlusionTexture;
        QString occlusionTextureID = occlusionTextures.value(material.materialID);
        if (occlusionTextureID.isNull()) {
            // 2nd chance
            // For blender we use the ambient factor texture as AOMap ONLY if the ambientFactor value is > 0.0
            if (material.ambientFactor > 0.0f) {
                occlusionTextureID = ambientFactorTextures.value(material.materialID);
            }
        }

        if (!occlusionTextureID.isNull()) {
            occlusionTexture = getTexture(occlusionTextureID, material.materialID);
            detectDifferentUVs |= (occlusionTexture.texcoordSet != 0) || (!emissiveTexture.transform.isIdentity());
            material.occlusionTexture = occlusionTexture;
        }

        glm::vec2 lightmapParams(0.f, 1.f);
        lightmapParams.x = _lightmapOffset;
        lightmapParams.y = _lightmapLevel;

        HFMTexture ambientTexture;
        QString ambientTextureID = ambientTextures.value(material.materialID);
        if (ambientTextureID.isNull()) {
            // 2nd chance
            // For blender we use the ambient factor texture as Lightmap ONLY if the ambientFactor value is set to 0
            if (material.ambientFactor == 0.0f) {
                ambientTextureID = ambientFactorTextures.value(material.materialID);
            }
        }

        if (_loadLightmaps && !ambientTextureID.isNull()) {
            ambientTexture = getTexture(ambientTextureID, material.materialID);
            detectDifferentUVs |= (ambientTexture.texcoordSet != 0) || (!ambientTexture.transform.isIdentity());
            material.lightmapTexture = ambientTexture;
            material.lightmapParams = lightmapParams;
        }

        // Finally create the true material representation
        material._material = std::make_shared<graphics::Material>();

        // Emissive color is the mix of emissiveColor with emissiveFactor
        auto emissive = material.emissiveColor *
                        (isMaterialLambert ? 1.0f : material.emissiveFactor); // In lambert there is not emissiveFactor
        material._material->setEmissive(emissive);

        // Final diffuse color is the mix of diffuseColor with diffuseFactor
        auto diffuse = material.diffuseColor * material.diffuseFactor;
        material._material->setAlbedo(diffuse);

        if (material.isPBSMaterial) {
            material._material->setRoughness(material.roughness);
            material._material->setMetallic(material.metallic);
        } else {
            material._material->setRoughness(graphics::Material::shininessToRoughness(material.shininess));
            float metallic = std::max(material.specularColor.x, std::max(material.specularColor.y, material.specularColor.z));
            material._material->setMetallic(metallic);

            if (isMaterialLambert) {
                if (!material._material->getKey().isAlbedo()) {
                    // switch emissive to material albedo as we tag the material to unlit
                    material._material->setUnlit(true);
                    material._material->setAlbedo(emissive);

                    if (!material.emissiveTexture.isNull()) {
                        material.albedoTexture = material.emissiveTexture;
                    }
                }
            }
        }
        qCDebug(modelformat) << " fbx material Name:" << material.name;

        if (material.opacity <= 0.0f) {
            material._material->setOpacity(1.0f);
        } else {
            material._material->setOpacity(material.opacity);
        }
    }
}
