#include <gccore.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "things.h"

void PreprocessVerts(AnimatedThing* thing) {
	if (thing->blendedvertices == NULL) return;
	unsigned int finalcount = thing->numvertices;
	thing->vertices = (MATRIXVERTEX*)malloc(sizeof(MATRIXVERTEX) * finalcount);
	for (int i = 0; i < thing->numvertices; i++) {
		SKINNED_VERTEX* sv = &thing->blendedvertices[i];
		MATRIXVERTEX* mv = &thing->vertices[i];
		
		// find highest weight bone
		u8 dominantbone = 0;
		f32 maxweight = 0.0f;

		for (int b = 0; b < 4; b++) {
			if (sv->weights[b] > maxweight) {
				maxweight = sv->weights[b];
				dominantbone = sv->indices[b];
			}
		}
		if (dominantbone >= thing->armature.numBones) // malformed data
			dominantbone = 0;
		if (dominantbone >= 9) // oh shit the bone index is more than the max supported by gx, lets just fallback to 9
			dominantbone = 9;
		mv->x = sv->x;
		mv->y = sv->y;
		mv->z = sv->z;
		mv->nx = sv->nx;
		mv->ny = sv->ny;
		mv->nz = sv->nz;
		mv->u = sv->u;
		mv->v = sv->v;
		mv->index = dominantbone;
	}
}
void FreeAnimatedThing(AnimatedThing *model) {
    if (model->vertices) free(model->vertices);
    if (model->blendedvertices) free(model->blendedvertices);
    if (model->indices) free(model->indices);
    
    if (model->armature.bones) free(model->armature.bones);
    if (model->armature.bindPoseInverse) free(model->armature.bindPoseInverse);
    
    if (model->texs) {
        for (int i = 0; i < model->numtexs; i++) {
            free(model->texs[i].data);
        }
        free(model->texs);
    }
    
    if (model->objs) free(model->objs);
    
    for (int a = 0; a < model->numanims; a++) {
        Animation* anim = &model->animations[a];
        for (int t = 0; t < anim->numtracks; t++) {
            if (anim->tracks[t].keyframes) {
                free(anim->tracks[t].keyframes);
            }
        }
        if (anim->tracks) free(anim->tracks);
    }
    if (model->animations) free(model->animations);
    
    memset(model, 0, sizeof(AnimatedThing));
}
void UpdateBoneTransforms(AnimatedThing *model) {
	Armature* armature = &model->armature;
	for (int i = 0; i < armature->numBones; i++) {
		Bone* bone = &armature->bones[i];
		Mtx trans, rotX, rotY, rotZ, temp, temp2;

		guMtxIdentity(trans);
		guMtxTransApply(trans, trans, bone->pos.x, bone->pos.y, bone->pos.z);
		guVector Xaxis = {1, 0, 0};	
		guVector Yaxis = {0, 1, 0};
		guVector Zaxis = {0, 0, 1};
		guMtxRotAxisDeg(rotX, &Xaxis, bone->rot.x);
		guMtxRotAxisDeg(rotY, &Yaxis, bone->rot.y);
		guMtxRotAxisDeg(rotZ, &Zaxis, bone->rot.z);
		guMtxConcat(rotZ, rotY, temp);
		guMtxConcat(temp, rotX, temp2);
		guMtxConcat(trans, temp2, bone->localTransform);
		if (bone->scale.x != 1.0f || bone->scale.y != 1.0f || bone->scale.z != 1.0f) {
            		Mtx scale;
            		guMtxScale(scale, bone->scale.x, bone->scale.y, bone->scale.z);
            		guMtxConcat(bone->localTransform, scale, temp);
            		guMtxCopy(temp, bone->localTransform);
        	}
		if (bone->parent >= 0) {
            		guMtxConcat(armature->bones[bone->parent].worldTransform,
        		bone->localTransform,
                	bone->worldTransform);
        	} else
            		guMtxCopy(bone->localTransform, bone->worldTransform);
	}
}
void UpdateAnimation(AnimatedThing *model, AnimState *state) {
	if (!state->playing || state->current >= model->numanims) return;
	Animation* anim = &model->animations[state->current];
	state->time += deltaTime * state->speed;
	if (state->time >= anim->length) {
        	if (state->looping) {
            		state->time = fmodf(state->time, anim->length);
        	} else {
            		state->time = anim->length;
            		state->playing = false;
            		return;
        	}
    	}
	for (int i = 0; i < anim->numtracks; i++) {
		AnimTrack* track = &anim->tracks[i];

		if(track->boneIndex >= model->armature.numBones || track->numkeyframes < 2) continue;

		Bone* bone = &model->armature.bones[track->boneIndex];
		int kf0, kf1;
		kf0 = kf1 = 0;
		for (int k = 0; k < track->numkeyframes - 1; k++) {
			if (state->time >= track->keyframes[k].time && state->time <= track->keyframes[k + 1].time) {
				kf0 = k;
				kf1 = k + 1;
				break;
			}
		}
		Keyframe* k0 = &track->keyframes[kf0];
		Keyframe* k1 = &track->keyframes[kf1];
		// its interpolatin' time
		f32 ratio = 0.0f;
			
		if (k1->time > k0->time) {
			ratio = (state->time - k0->time) / (k1->time - k0->time);
			// gee i sure do hope this code wont have any severe performance issues due to my lack of knowledge of the PowerPC ISA
			f32 temp = ratio < 0.0f ? 0.0f : ratio;
			ratio = temp > 1.0f ? 1.0f : temp;
		}
		
		// its interpolatin' time for real now
		// this works by using linear interpolation to get a value inbetween the two keyframes via using that ratio float we just calculated as a position, 1.0 being k1's value and 0.0 being k0's value 
		bone->pos.x = k0->pos.x + (k1->pos.x - k0->pos.x) * ratio;
		bone->pos.y = k0->pos.y + (k1->pos.y - k0->pos.y) * ratio;
		bone->pos.z = k0->pos.z + (k1->pos.z - k0->pos.z) * ratio;

		bone->rot.x = k0->rot.x + (k1->rot.x - k0->rot.x) * ratio;
		bone->rot.y = k0->rot.y + (k1->rot.y - k0->rot.y) * ratio;
		bone->rot.z = k0->rot.z + (k1->rot.z - k0->rot.z) * ratio;
		
		bone->scale.x = k0->scale.x + (k1->scale.x - k0->scale.x) * ratio;
		bone->scale.y = k0->scale.y + (k1->scale.y - k0->scale.y) * ratio;
		bone->scale.z = k0->scale.z + (k1->scale.z - k0->scale.z) * ratio;
	}
	UpdateBoneTransforms(model);
}
void DrawAnimatedThing(Mtx v, AnimatedThing *model, f32 posX, f32 posY, f32 posZ) {
	// malformed
	if (!model->vertices || model->numvertices == 0) return;
	Mtx m, mr, mt, mv;
	guVector Xaxis = {1.0f, 0, 0};
    	guVector Yaxis = {0, 1.0f, 0};
	f32 xtrans = -xpos;
    	f32 ztrans = -zpos;
    	f32 ytrans = -walkbias - 0.25f;
    	f32 scenerotx = 360.0f - xrot;
    	f32 sceneroty = 360.0f - yrot;

	guMtxIdentity(m);
	guMtxRotAxisDeg(m, &Xaxis, scenerotx);
	guMtxRotAxisDeg(m, &Yaxis, sceneroty);
	guMtxConcat(m, v, mv);

	guMtxApplyTrans(mv, mt, xtrans, ytrans, ztrans);
	guMtxApplyTrans(mt, m, posX, posY, posZ);
	guMtxConcat(m, mr, mv);
	u8 maxBones = model->armature.numBones < 10 ? model->armature.numBones : 10;
	for (int i = 0; i < maxBones; i++) {
        	Bone* bone = &model->armature.bones[i];
        	Mtx skinMtx, finalMtx;
        	guMtxConcat(bone->worldTransform, model->armature.bindPoseInverse[i], skinMtx);
        
        	guMtxConcat(mv, skinMtx, finalMtx);
        	// damm you preprocessor defines	
        	GX_LoadPosMtxImm(finalMtx, GX_PNMTX0 + (i * 3));
    	}
	if (model->numtexs > 0 && model->objs != NULL)
        	GX_LoadTexObj(&model->objs[0], GX_TEXMAP0);
	GX_ClearVtxDesc();
    	GX_SetVtxDesc(GX_VA_PTNMTXIDX, GX_DIRECT);
    	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    	GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT);
    	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	for (int t = 0; t < model->numvertices; t++) {
        	GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
        
        	for (int v = 0; v < 3; v++) {
        		int idx = model->indices[t * 3 + v];
            		MATRIXVERTEX* vert = &model->vertices[idx];
            		GX_MatrixIndex1x8(vert->index);
            		GX_Position3f32(vert->x, vert->y, vert->z);
            		GX_Normal3f32(vert->nx, vert->ny, vert->nz);
            		GX_TexCoord2f32(vert->u, vert->v);
        	}
        	GX_End();
    	}
	GX_ClearVtxDesc();
    	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    	GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT);
    	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

}
