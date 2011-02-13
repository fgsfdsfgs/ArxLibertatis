/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// EERIEMeshTweak
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		DynMeshTweaking
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "graphics/data/MeshManipulation.h"

#include <cstdio>

#include "core/Application.h"
#include "scene/Object.h"
#include "graphics/Math.h"

#include "io/IO.h"
#include "io/PakManager.h"
#include "io/Logger.h"

using std::max;

void EERIE_MESH_ReleaseTransPolys(EERIE_3DOBJ * obj)
{
	if (!obj) return;

	for (long i = 0; i < INTERTRANSPOLYSPOS; i++)
	{
		for (size_t ii = 0; ii < obj->texturecontainer.size(); ii++)
		{
			if (InterTransTC[i] == obj->texturecontainer[ii])
			{
				for (size_t jj = 0; jj < obj->facelist.size(); jj++)
				{
					if (InterTransFace[jj] == &obj->facelist[jj])
						InterTransFace[jj] = NULL;
				}
			}
		}
	}
}
 
void EERIE_MESH_TWEAK_Skin(EERIE_3DOBJ * obj, const std::string& s1, const std::string& s2)
{
	LogDebug << "Tweak Skin " << s1 << " " << s2;

	if ( obj == NULL || s1.empty() || s2.empty() )
	{
		LogError << "Tweak Skin got NULL Pointer";
		return;
	}
	
	LogDebug << "Tweak Skin " << s1 << " " << s2;

	std::string skintochange = "Graph\\Obj3D\\Textures\\" + s1 + ".bmp";
	MakeUpcase(skintochange);
	TextureContainer * tex = D3DTextr_CreateTextureFromFile(skintochange);

	if (obj->originaltextures == NULL)
	{
		obj->originaltextures = (char *)malloc(256 * obj->texturecontainer.size()); 
		memset(obj->originaltextures, 0, 256 * obj->texturecontainer.size());

		for (size_t i = 0; i < obj->texturecontainer.size(); i++)
		{
			if (obj->texturecontainer[i])
				strcpy(obj->originaltextures + 256 * i, obj->texturecontainer[i]->m_texName.c_str());

		}
	}

	if ((tex != NULL) && (obj->originaltextures != NULL))
	{
		for (size_t i = 0; i < obj->texturecontainer.size(); i++)
		{
			if ((strstr(obj->originaltextures + 256 * i, skintochange.c_str())))
			{
				skintochange = obj->texturecontainer[i]->m_texName;
				break;
			}
		}

		tex->Restore(GDevice);
		TextureContainer * tex2 = FindTexture(skintochange);

		if (tex2)
			for (size_t i = 0; i < obj->texturecontainer.size(); i++)
			{
				if (obj->texturecontainer[i] == tex2)  obj->texturecontainer[i] = tex;
			}
	}
}

//**************************************************************************************************************
//**************************************************************************************************************
long IsInSelection(EERIE_3DOBJ * obj, long vert, long tw)
{
	if (obj == NULL) return -1;

	if (tw < 0) return -1;

	if (vert < 0) return -1;

	for (long i = 0; i < obj->selections[tw].selected.size(); i++)
	{
		if (obj->selections[tw].selected[i] == vert)
			return i;
	}

	return -1;
}
long GetEquivalentVertex(EERIE_3DOBJ * obj, EERIE_VERTEX * vert)
{
	for (size_t i = 0; i < obj->vertexlist.size(); i++)
	{
		if ((obj->vertexlist[i].v.x == vert->v.x)
		        &&	(obj->vertexlist[i].v.y == vert->v.y)
		        &&	(obj->vertexlist[i].v.z == vert->v.z))
			return i;
	}

	return -1;
}

//*************************************************************************************
//*************************************************************************************
long ObjectAddVertex(EERIE_3DOBJ * obj, EERIE_VERTEX * vert)
{
	for (size_t i = 0; i < obj->vertexlist.size(); i++)
	{
		if ((obj->vertexlist[i].v.x == vert->v.x)  
		        &&	(obj->vertexlist[i].v.y == vert->v.y) 
		        &&	(obj->vertexlist[i].v.z == vert->v.z)) 
			return i;
	}

	obj->vertexlist.push_back(*vert);
	return (obj->vertexlist.size() - 1);
}
//*************************************************************************************
//*************************************************************************************
long GetActionPoint(EERIE_3DOBJ * obj, const char * name)
{
	if (!obj) return -1;

	for (size_t n = 0; n < obj->actionlist.size(); n++)
	{ // TODO iterator
		if (!strcasecmp(obj->actionlist[n].name.c_str(), name))
			return obj->actionlist[n].idx;
	}

	return -1;
}
//*************************************************************************************
//*************************************************************************************
long ObjectAddFace(EERIE_3DOBJ * obj, EERIE_FACE * face, EERIE_3DOBJ * srcobj)
{

	for (size_t i = 0; i < obj->facelist.size(); i++) // Check Already existing faces
	{
		if ((obj->vertexlist[obj->facelist[i].vid[0]].v.x == srcobj->vertexlist[face->vid[0]].v.x)
		        &&	(obj->vertexlist[obj->facelist[i].vid[1]].v.x == srcobj->vertexlist[face->vid[1]].v.x)
		        &&	(obj->vertexlist[obj->facelist[i].vid[2]].v.x == srcobj->vertexlist[face->vid[2]].v.x)
		        &&	(obj->vertexlist[obj->facelist[i].vid[0]].v.y == srcobj->vertexlist[face->vid[0]].v.y)
		        &&	(obj->vertexlist[obj->facelist[i].vid[1]].v.y == srcobj->vertexlist[face->vid[1]].v.y)
		        &&	(obj->vertexlist[obj->facelist[i].vid[2]].v.y == srcobj->vertexlist[face->vid[2]].v.y)
		        &&	(obj->vertexlist[obj->facelist[i].vid[0]].v.z == srcobj->vertexlist[face->vid[0]].v.z)
		        &&	(obj->vertexlist[obj->facelist[i].vid[1]].v.z == srcobj->vertexlist[face->vid[1]].v.z)
		        &&	(obj->vertexlist[obj->facelist[i].vid[2]].v.z == srcobj->vertexlist[face->vid[2]].v.z)
		   )

			return -1;
	}

	long f0, f1, f2;
	f0 = ObjectAddVertex(obj, &srcobj->vertexlist[face->vid[0]]);
	f1 = ObjectAddVertex(obj, &srcobj->vertexlist[face->vid[1]]);
	f2 = ObjectAddVertex(obj, &srcobj->vertexlist[face->vid[2]]);

	if ((f1 == -1) || (f2 == -1) || (f0 == -1)) return -1;

	
	obj->facelist.push_back(*face);

	obj->facelist.back().vid[0] = (unsigned short)f0;
	obj->facelist.back().vid[1] = (unsigned short)f1;
	obj->facelist.back().vid[2] = (unsigned short)f2;
	obj->facelist.back().texid = 0; 

	for (size_t i = 0; i < obj->texturecontainer.size(); i++)
	{
		if (obj->texturecontainer[i] == srcobj->texturecontainer[face->texid])
		{
			obj->facelist.back().texid = (short)i;
			break;
		}
	}

	return (obj->facelist.size() - 1);
}
//*************************************************************************************
//*************************************************************************************
long ObjectAddAction(EERIE_3DOBJ * obj, const char * name, long act,
                     long sfx, EERIE_VERTEX * vert) 
{
	long newvert = ObjectAddVertex(obj, vert);

	if (newvert < 0) return -1;
	
	long j = 0;
	for(vector<EERIE_ACTIONLIST>::iterator i = obj->actionlist.begin();
	    i != obj->actionlist.end(); ++i) {
		if(!i->name.compare(name)) {
			return j;
		}
		j++;
	}
	
	obj->actionlist.push_back(EERIE_ACTIONLIST());
	
	EERIE_ACTIONLIST & action = obj->actionlist.back();
	
	action.name = name;
	action.act = act;
	action.sfx = sfx;
	action.idx = newvert;
	
	return (obj->actionlist.size() - 1);
}
//*************************************************************************************
//*************************************************************************************
long ObjectAddMap(EERIE_3DOBJ * obj, TextureContainer * tc)
{
	if (tc == NULL) return -1;

	for (size_t i = 0; i < obj->texturecontainer.size(); i++)
	{
		if (obj->texturecontainer[i] == tc)
			return i;
	}

	obj->texturecontainer.push_back(tc);

	return (obj->texturecontainer.size() - 1);
}
//*************************************************************************************
//*************************************************************************************
void AddVertexToGroup(EERIE_3DOBJ * obj, long group, EERIE_VERTEX * vert)
{

	for (size_t i = 0; i < obj->vertexlist.size(); i++)
	{
		if ((obj->vertexlist[i].v.x == vert->v.x)
		        &&	(obj->vertexlist[i].v.y == vert->v.y)
		        &&	(obj->vertexlist[i].v.z == vert->v.z))
		{
			AddVertexIdxToGroup(obj, group, i);
		}
	}
}

void AddVertexIdxToGroupWithoutInheritance(EERIE_3DOBJ * obj, long group, long val) {
	
	for(long i = 0; i < obj->grouplist[group].indexes.size(); i++) {
		if(obj->grouplist[group].indexes[i] == val) {
			return;
		}
	}
	
	obj->grouplist[group].indexes.push_back(val);
}

void AddVertexIdxToGroup(EERIE_3DOBJ * obj, long group, long val)
{
	AddVertexIdxToGroupWithoutInheritance(obj, group, val);
}

void ObjectAddSelection(EERIE_3DOBJ * obj, long numsel, long vidx)
{
	
	for (long i = 0; i < obj->selections[numsel].selected.size(); i++)
	{
		if (obj->selections[numsel].selected[i] == vidx) return;
	}
	
	obj->selections[numsel].selected.push_back(vidx);
}

//*************************************************************************************
//*************************************************************************************
EERIE_3DOBJ * CreateIntermediaryMesh(EERIE_3DOBJ * obj1, EERIE_3DOBJ * obj2, long tw)
{
	long tw1 = -1;
	long tw2 = -1;
	long iw1 = -1;
	long iw2 = -1;
	long jw1 = -1;
	long jw2 = -1;
	long sel_head1 = -1;
	long sel_head2 = -1;
	long sel_torso1 = -1;
	long sel_torso2 = -1;
	long sel_legs1 = -1;
	long sel_legs2 = -1;

	// First we retreive selection groups indexes
	for (size_t i = 0; i < obj1->selections.size(); i++)
	{ // TODO iterator
		if (!strcasecmp(obj1->selections[i].name.c_str(), "head")) sel_head1 = i;
		else if (!strcasecmp(obj1->selections[i].name.c_str(), "chest")) sel_torso1 = i;
		else if (!strcasecmp(obj1->selections[i].name.c_str(), "leggings")) sel_legs1 = i;
	}

	for (size_t i = 0; i < obj2->selections.size(); i++)
	{ // TODO iterator
		if (!strcasecmp(obj2->selections[i].name.c_str(), "head")) sel_head2 = i;
		else if (!strcasecmp(obj2->selections[i].name.c_str(), "chest")) sel_torso2 = i;
		else if (!strcasecmp(obj2->selections[i].name.c_str(), "leggings")) sel_legs2 = i;
	}

	if (sel_head1 == -1) return NULL;

	if (sel_head2 == -1) return NULL;

	if (sel_torso1 == -1) return NULL;

	if (sel_torso2 == -1) return NULL;

	if (sel_legs1 == -1) return NULL;

	if (sel_legs2 == -1) return NULL;

	if (tw == TWEAK_HEAD)
	{
		tw1 = sel_head1;
		tw2 = sel_head2;
		iw1 = sel_torso1;
		iw2 = sel_torso2;
		jw1 = sel_legs1;
		jw2 = sel_legs2;
	}

	if (tw == TWEAK_TORSO)
	{
		tw1 = sel_torso1;
		tw2 = sel_torso2;
		iw1 = sel_head1;
		iw2 = sel_head2;
		jw1 = sel_legs1;
		jw2 = sel_legs2;
	}

	if (tw == TWEAK_LEGS)
	{
		tw1 = sel_legs1;
		tw2 = sel_legs2;
		iw1 = sel_torso1;
		iw2 = sel_torso2;
		jw1 = sel_head1;
		jw2 = sel_head2;
	}

	if ((tw1 == -1) || (tw2 == -1)) return NULL;

	// Now Retreives Tweak Action Points
	{
		long idx_head1, idx_head2;
		long idx_torso1, idx_torso2;
		long idx_legs1, idx_legs2;

		idx_head1 = GetActionPoint(obj1, "head2chest");

		if (idx_head1 < 0) return NULL;

		idx_head2 = GetActionPoint(obj2, "head2chest");

		if (idx_head2 < 0) return NULL;

		idx_torso1 = GetActionPoint(obj1, "chest2leggings");

		if (idx_torso1 < 0) return NULL;

		idx_torso2 = GetActionPoint(obj2, "chest2leggings");

		if (idx_torso2 < 0) return NULL;

		idx_legs1 = obj1->origin;
		idx_legs2 = obj2->origin;

	}

	// copy vertices
	vector<EERIE_VERTEX> obj1vertexlist2 = obj1->vertexlist;
	vector<EERIE_VERTEX> obj2vertexlist2 = obj2->vertexlist;

	// Work will contain the Tweaked object
	EERIE_3DOBJ * work = new EERIE_3DOBJ;
	// TODO string breaker
	memcpy(&work->pos, &obj1->pos, sizeof(EERIE_3D));
	memcpy(&work->angle, &obj1->angle, sizeof(EERIE_3D));

	// ident will be the same as original object obj1
	work->ident = obj1->ident;

	// We reset all data to create a fresh object
	memcpy(&work->cub, &obj1->cub, sizeof(CUB3D));
	memcpy(&work->quat, &obj1->quat, sizeof(EERIE_QUAT));

	// Linked objects are linked to this object.
	if (obj1->nblinked > obj2->nblinked)
	{
		work->linked = (EERIE_LINKED *)malloc(obj1->nblinked * sizeof(EERIE_LINKED));
		memcpy(work->linked, obj1->linked, obj1->nblinked * sizeof(EERIE_LINKED));
		work->nblinked = obj1->nblinked;
	}
	else if (obj2->nblinked > 0)
	{
		work->linked = (EERIE_LINKED *)malloc(obj2->nblinked * sizeof(EERIE_LINKED));
		memcpy(work->linked, obj2->linked, obj2->nblinked * sizeof(EERIE_LINKED));
		work->nblinked = obj2->nblinked;
	}
	else
	{
		work->linked = NULL;
		work->nblinked = 0;
	}

	// Is the origin of object in obj1 or obj2 ? Retreives it for work object
	if (IsInSelection(obj1, obj1->origin, tw1) != -1)
	{
		work->point0.x = obj2->point0.x;
		work->point0.y = obj2->point0.y;
		work->point0.z = obj2->point0.z;
		work->origin = ObjectAddVertex(work, &obj2vertexlist2[obj2->origin]); 
	}
	else
	{
		work->point0.x = obj1->point0.x;
		work->point0.y = obj1->point0.y;
		work->point0.z = obj1->point0.z;
		work->origin = ObjectAddVertex(work, &obj1vertexlist2[obj1->origin]); 
	}

	// Recreate Action Points included in work object.for Obj1
	for (size_t i = 0; i < obj1->actionlist.size(); i++)
	{
		if ((IsInSelection(obj1, obj1->actionlist[i].idx, iw1) != -1)
		        ||	(IsInSelection(obj1, obj1->actionlist[i].idx, jw1) != -1)
		        || (!strcasecmp(obj1->actionlist[i].name.c_str(), "head2chest"))
		        || (!strcasecmp(obj1->actionlist[i].name.c_str(), "chest2leggings"))
		   )
		{
			ObjectAddAction(work, obj1->actionlist[i].name.c_str(), obj1->actionlist[i].act,
			                obj1->actionlist[i].sfx, &obj1vertexlist2[obj1->actionlist[i].idx]);
		}
	}

	// Do the same for Obj2
	for (size_t i = 0; i < obj2->actionlist.size(); i++)
	{
		if ((IsInSelection(obj2, obj2->actionlist[i].idx, tw2) != -1)
		        || (!strcasecmp(obj1->actionlist[i].name.c_str(), "head2chest"))
		        || (!strcasecmp(obj1->actionlist[i].name.c_str(), "chest2leggings"))
		   )
		{
			ObjectAddAction(work, obj2->actionlist[i].name.c_str(), obj2->actionlist[i].act,
			                obj2->actionlist[i].sfx, &obj2vertexlist2[obj2->actionlist[i].idx]);
		}
	}

	// Recreate Vertex using Obj1 Vertexes
	for (size_t i = 0; i < obj1->vertexlist.size(); i++)
	{
		if ((IsInSelection(obj1, i, iw1) != -1)
		        ||	(IsInSelection(obj1, i, jw1) != -1))
		{
			ObjectAddVertex(work, &obj1vertexlist2[i]);
		}
	}

	// The same for Obj2
	for (size_t i = 0; i < obj2->vertexlist.size(); i++)
	{
		if (IsInSelection(obj2, i, tw2) != -1)
		{
			ObjectAddVertex(work, &obj2vertexlist2[i]);
		}
	}


	// Look in Faces for forgotten Vertexes... AND
	// Re-Create TextureContainers Infos
	// We look for texturecontainers included in the future tweaked object
	TextureContainer * tc = NULL;

	for (size_t i = 0; i < obj1->facelist.size(); i++)
	{
		if (((IsInSelection(obj1, obj1->facelist[i].vid[0], iw1) != -1)
		        ||	(IsInSelection(obj1, obj1->facelist[i].vid[0], jw1) != -1))
		        &&	((IsInSelection(obj1, obj1->facelist[i].vid[1], iw1) != -1)
		             ||	(IsInSelection(obj1, obj1->facelist[i].vid[1], jw1) != -1))
		        &&	((IsInSelection(obj1, obj1->facelist[i].vid[2], iw1) != -1)
		             ||	(IsInSelection(obj1, obj1->facelist[i].vid[2], jw1) != -1))
		   )
		{

			if (obj1->facelist[i].texid != -1)
				if (tc != obj1->texturecontainer[obj1->facelist[i].texid])
				{
					tc = obj1->texturecontainer[obj1->facelist[i].texid];
					ObjectAddMap(work, tc);
				}

			ObjectAddFace(work, &obj1->facelist[i], obj1);
		}
	}

	for (size_t i = 0; i < obj2->facelist.size(); i++)
	{
		if ((IsInSelection(obj2, obj2->facelist[i].vid[0], tw2) != -1)
		        || (IsInSelection(obj2, obj2->facelist[i].vid[1], tw2) != -1)
		        || (IsInSelection(obj2, obj2->facelist[i].vid[2], tw2) != -1))
		{

			if (obj2->facelist[i].texid != -1)
				if (tc != obj2->texturecontainer[obj2->facelist[i].texid])
				{
					tc = obj2->texturecontainer[obj2->facelist[i].texid];
					ObjectAddMap(work, tc);
				}

			ObjectAddFace(work, &obj2->facelist[i], obj2);
		}
	}

	// Recreate Groups
	work->nbgroups = std::max(obj1->nbgroups, obj2->nbgroups);
	work->grouplist = new EERIE_GROUPLIST[work->nbgroups];

	for (long k = 0; k < obj1->nbgroups; k++)
	{
		work->grouplist[k].name = obj1->grouplist[k].name;
		long v = GetEquivalentVertex(work, &obj1vertexlist2[obj1->grouplist[k].origin]);

		if (v >= 0)
		{
			work->grouplist[k].siz = obj1->grouplist[k].siz;

			if ((IsInSelection(obj1, obj1->grouplist[k].origin, iw1) != -1)
			        || (IsInSelection(obj1, obj1->grouplist[k].origin, jw1) != -1))
				work->grouplist[k].origin = v;
		}
	}

	for (int k = 0; k < obj2->nbgroups; k++)
	{
		if (k >= obj1->nbgroups)
		{
			work->grouplist[k].name = obj2->grouplist[k].name;

		}

		long v = GetEquivalentVertex(work, &obj2vertexlist2[obj2->grouplist[k].origin]);

		if (v >= 0)
		{
			work->grouplist[k].siz = obj2->grouplist[k].siz;

			if (IsInSelection(obj2, obj2->grouplist[k].origin, tw2) != -1)
				work->grouplist[k].origin = v;
		}
	}

	// Recreate Selection Groups (only the 3 selections needed to reiterate MeshTweaking !)
	work->selections.resize(3);
	work->selections[0].name = "head";
	work->selections[1].name = "chest";
	work->selections[2].name = "leggings";

	// Re-Creating sel_head
	if (tw == TWEAK_HEAD)
	{
		for (long l = 0; l < obj2->selections[sel_head2].selected.size(); l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj2vertexlist2[obj2->selections[sel_head2].selected[l]].v.x;
			temp.v.y = obj2vertexlist2[obj2->selections[sel_head2].selected[l]].v.y;
			temp.v.z = obj2vertexlist2[obj2->selections[sel_head2].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 0, t);
			}
		}
	}
	else for (long l = 0; l < obj1->selections[sel_head1].selected.size(); l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj1vertexlist2[obj1->selections[sel_head1].selected[l]].v.x;
			temp.v.y = obj1vertexlist2[obj1->selections[sel_head1].selected[l]].v.y;
			temp.v.z = obj1vertexlist2[obj1->selections[sel_head1].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 0, t);
			}
		}

	// Re-Create sel_torso
	if (tw == TWEAK_TORSO)
	{
		for (long l = 0; l < obj2->selections[sel_torso2].selected.size(); l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj2vertexlist2[obj2->selections[sel_torso2].selected[l]].v.x;
			temp.v.y = obj2vertexlist2[obj2->selections[sel_torso2].selected[l]].v.y;
			temp.v.z = obj2vertexlist2[obj2->selections[sel_torso2].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 1, t);
			}
		}
	}
	else for (long l = 0; l < obj1->selections[sel_torso1].selected.size(); l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj1vertexlist2[obj1->selections[sel_torso1].selected[l]].v.x;
			temp.v.y = obj1vertexlist2[obj1->selections[sel_torso1].selected[l]].v.y;
			temp.v.z = obj1vertexlist2[obj1->selections[sel_torso1].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 1, t);
			}
		}

	// Re-Create sel_legs
	if (tw == TWEAK_LEGS)
	{
		for (long l = 0; l < obj2->selections[sel_legs2].selected.size(); l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj2vertexlist2[obj2->selections[sel_legs2].selected[l]].v.x;
			temp.v.y = obj2vertexlist2[obj2->selections[sel_legs2].selected[l]].v.y;
			temp.v.z = obj2vertexlist2[obj2->selections[sel_legs2].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 2, t);
			}
		}
	}
	else for (long l = 0; l < obj1->selections[sel_legs1].selected.size(); l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj1vertexlist2[obj1->selections[sel_legs1].selected[l]].v.x;
			temp.v.y = obj1vertexlist2[obj1->selections[sel_legs1].selected[l]].v.y;
			temp.v.z = obj1vertexlist2[obj1->selections[sel_legs1].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 2, t);
			}
		}

	//Now recreates other selections...
	for (size_t i = 0; i < obj1->selections.size(); i++)
	{ // TODO iterator
		if (EERIE_OBJECT_GetSelection(work, obj1->selections[i].name.c_str()) == -1)
		{
			long num = work->selections.size();
			work->selections.resize(num + 1);
			work->selections[num].name = obj1->selections[i].name;

			for (long l = 0; l < obj1->selections[i].selected.size(); l++)
			{
				EERIE_VERTEX temp;
				temp.v.x = obj1vertexlist2[obj1->selections[i].selected[l]].v.x;
				temp.v.y = obj1vertexlist2[obj1->selections[i].selected[l]].v.y;
				temp.v.z = obj1vertexlist2[obj1->selections[i].selected[l]].v.z;
				long t = GetEquivalentVertex(work, &temp);

				if (t != -1)
				{
					ObjectAddSelection(work, num, t);
				}
			}

			long ii = EERIE_OBJECT_GetSelection(obj2, obj1->selections[i].name.c_str());

			if (ii != -1)
				for (long l = 0; l < obj2->selections[ii].selected.size(); l++)
				{
					EERIE_VERTEX temp;
					temp.v.x = obj2vertexlist2[obj2->selections[ii].selected[l]].v.x;
					temp.v.y = obj2vertexlist2[obj2->selections[ii].selected[l]].v.y;
					temp.v.z = obj2vertexlist2[obj2->selections[ii].selected[l]].v.z;
					long t = GetEquivalentVertex(work, &temp);

					if (t != -1)
					{
						ObjectAddSelection(work, num, t);
					}
				}
		}
	}

	for (size_t i = 0; i < obj2->selections.size(); i++)
	{ // TODO iterator
		if (EERIE_OBJECT_GetSelection(work, obj2->selections[i].name.c_str()) == -1)
		{
			long num = work->selections.size();
			work->selections.resize(num + 1);
			work->selections[num].name = obj2->selections[i].name;

			for (long l = 0; l < obj2->selections[i].selected.size(); l++)
			{
				EERIE_VERTEX temp;
				temp.v.x = obj2vertexlist2[obj2->selections[i].selected[l]].v.x;
				temp.v.y = obj2vertexlist2[obj2->selections[i].selected[l]].v.y;
				temp.v.z = obj2vertexlist2[obj2->selections[i].selected[l]].v.z;
				long t = GetEquivalentVertex(work, &temp);

				if (t != -1)
				{
					ObjectAddSelection(work, num, t);
				}
			}
		}
	}

	// Recreate Animation-groups vertex
	for (long i = 0; i < obj1->nbgroups; i++)
	{
		for (long j = 0; j < obj1->grouplist[i].indexes.size(); j++)
		{
			AddVertexToGroup(work, i, &obj1vertexlist2[obj1->grouplist[i].indexes[j]]);
		}
	}

	for (long i = 0; i < obj2->nbgroups; i++)
	{
		for (long j = 0; j < obj2->grouplist[i].indexes.size(); j++)
		{
			AddVertexToGroup(work, i, &obj2vertexlist2[obj2->grouplist[i].indexes[j]]);
		}
	}

	// Look for Vertices Without Group...
	for (size_t i = 0; i < work->texturecontainer.size(); i++)
	{
		work->texturecontainer[i]->Restore(GDevice);
	}

	work->vertexlist3 = work->vertexlist;

	return work;
}
long ALLOW_MESH_TWEAKING = 1;

//*************************************************************************************
//*************************************************************************************

void EERIE_MESH_TWEAK_Do(INTERACTIVE_OBJ * io, long tw, const std::string& _path)
{
	if (!ALLOW_MESH_TWEAKING) return;

	std::string file2;
	std::string filet;
	std::string path;
	File_Standardize(_path, path);

	filet = "GAME\\" + path;

	SetExt(filet, ".FTL");
	File_Standardize(filet, file2);

	if ((!PAK_FileExist(file2.c_str())) && (!PAK_FileExist(path.c_str()))) return;

	if (tw == TWEAK_ERROR) return;

	if (io == NULL) return;

	if (io->obj == NULL) return;

	EERIE_MESH_ReleaseTransPolys(io->obj);

	if ( path.empty() && (tw == TWEAK_REMOVE))
	{
		if (io->tweaky)
		{
			ReleaseEERIE3DObj(io->obj);
			io->obj = io->tweaky;
			EERIE_Object_Precompute_Fast_Access(io->obj);
			io->tweaky = NULL;
		}

		return;
	}

	EERIE_3DOBJ * tobj = NULL;
	EERIE_3DOBJ * result = NULL;
	EERIE_3DOBJ * result2 = NULL;

	if ((PAK_FileExist(file2.c_str())) || (PAK_FileExist(path.c_str())))
	{
		const char tex1[] = "Graph\\Obj3D\\Textures\\";

		if (io->ioflags & IO_NPC)
			tobj = TheoToEerie_Fast(tex1, path.c_str(), TTE_NPC);
		else
			tobj = TheoToEerie_Fast(tex1, path.c_str(), 0);

		if (!tobj) return;

		switch (tw)
		{
			case TWEAK_ALL:

				if (io->tweaky == NULL) io->tweaky = io->obj;
				else ReleaseEERIE3DObj(io->obj);

				long i;
				TextureContainer * tc;
				tc = NULL;

				for (i = 0; i < tobj->facelist.size(); i++)
				{
					if ((tobj->facelist[i].texid >= 0)
					        &&	(tobj->texturecontainer[tobj->facelist[i].texid]))
					{
						tc = tobj->texturecontainer[tobj->facelist[i].texid];

						if (!tc->m_pddsSurface)
							tc->Restore(GDevice);
					}
				}

				io->obj = tobj;
				return;
				break;
			case TWEAK_UPPER:
				result2 = CreateIntermediaryMesh(io->obj, tobj, TWEAK_HEAD);
				result = CreateIntermediaryMesh(result2, tobj, TWEAK_TORSO);
				ReleaseEERIE3DObj(result2);
				break;
			case TWEAK_LOWER:
				result2 = CreateIntermediaryMesh(io->obj, tobj, TWEAK_TORSO);
				result = CreateIntermediaryMesh(result2, tobj, TWEAK_LEGS);
				ReleaseEERIE3DObj(result2);
				break;
			case TWEAK_UP_LO:
				result = CreateIntermediaryMesh(tobj, io->obj, TWEAK_TORSO);
				break;
			default:
				result = CreateIntermediaryMesh(io->obj, tobj, tw);
				break;
		}

		if (result == NULL)
		{
			ReleaseEERIE3DObj(tobj);
			return;
		}

		result->pdata = NULL;
		result->cdata = NULL;

		if (io->tweaky == NULL) io->tweaky = io->obj;
		else if (io->tweaky != io->obj)
			ReleaseEERIE3DObj(io->obj);

		io->obj = result;
		EERIE_Object_Precompute_Fast_Access(io->obj);
	}

	EERIE_CreateCedricData(io->obj);

	if (io)
	{
		io->lastanimtime = 0;
		io->nb_lastanimvertex = 0;
	}

	ReleaseEERIE3DObj(tobj);
}
