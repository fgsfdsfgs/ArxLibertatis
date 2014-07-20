/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
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

#include "cinematic/CinematicTexture.h"

#include <stddef.h>
#include <string>
#include <cstdlib>

#include <boost/foreach.hpp>

#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

static const int MaxW = 256;
static const int MaxH = 256;

using std::string;

void FreeGrille(CinematicGrid * grille);

static void ReajustUV(CinematicBitmap* bi);

CinematicBitmap::~CinematicBitmap()
{
	FreeGrille(&grid);
}

bool AllocGrille(CinematicGrid * grille, int nbx, int nby, float tx, float ty, float dx, float dy, int echelle)
{
	grille->echelle = echelle;
	int oldnbx = nbx + 1;
	int oldnby = nby + 1;
	nbx *= echelle;
	nby *= echelle;
	float olddx = dx;
	float olddy = dy;
	dx /= ((float)echelle);
	dy /= ((float)echelle);

	grille->nbuvs = 0;
	grille->nbinds = 0;
	grille->nbuvsmalloc = grille->nbvertexs = (nbx + 1) * (nby + 1);
	grille->nbfaces = (nbx * nby) << 1;
	grille->nbindsmalloc = grille->nbfaces;
	grille->vertexs = (Vec3f *)malloc(grille->nbvertexs * sizeof(Vec3f));
	grille->uvs = (C_UV *)malloc(grille->nbvertexs * sizeof(C_UV));
	grille->inds = (C_IND *)malloc(grille->nbindsmalloc * sizeof(C_IND));

	//vertexs
	grille->nbx = nbx;
	grille->nby = nby;
	tx *= .5f;
	ty *= .5f;
	float depx = -tx;
	float depy = -ty;
 
	Vec3f * v = grille->vertexs;
	float olddyy = olddy;

	while(oldnby--) {
		nby = echelle;

		if(!oldnby)
			nby = 1;

		while(nby--) {
			float olddxx = olddx;
			float depxx = depx;
			float dxx = dx;
			int oldnbxx = oldnbx;

			while(oldnbxx--) {
				nbx = echelle;

				if (!oldnbxx) nbx = 1;

				while(nbx--) {
					*v = Vec3f(depxx, depy, 0.f);
					depxx += dxx;
					v++;
				}

				olddxx += olddx;

				if(olddxx > (tx * 2.f)) {
					dxx = tx - depxx;
					dxx /= (float)echelle;
				}
				
			}

			depy += dy;
		}
		
			olddyy += olddy;

			if(olddyy > (ty * 2.f)) {
				dy = ty - depy;
				dy /= (float)echelle;
			}
		
	}

	return true;
}

int AddMaterial(CinematicGrid * grille, Texture2D* tex)
{
	int matIdx = grille->mats.size();
	grille->mats.resize(matIdx + 1);

	int deb;
	if(matIdx == 0)
		deb = 0;
	else
		deb = grille->mats[matIdx-1].startind + grille->mats[matIdx-1].nbind;

	grille->mats[matIdx].tex = tex;
	grille->mats[matIdx].startind = deb;
	grille->mats[matIdx].nbind = 0;

	return matIdx;
}

void FreeGrille(CinematicGrid * grille) {
	
	free(grille->vertexs);
	grille->vertexs = NULL;
	free(grille->uvs);
	grille->uvs = NULL;
	free(grille->inds);
	grille->inds = NULL;
	
	BOOST_FOREACH(C_INDEXED & mat, grille->mats) {
		delete mat.tex;
	}
	
	grille->mats.clear();
}

void GetIndNumCube(CinematicGrid * grille, int cx, int cy, int * i1, int * i2, int * i3, int * i4)
{
	*i1 = cy * (grille->nbx + 1) + cx;
	*i2 = *i1 + 1;
	*i3 = *i1 + (grille->nbx + 1);
	*i4 = *i3 + 1;
}

void AddPoly(CinematicGrid * grille, int matIdx, int i0, int i1, int i2) {
	
	if(grille->nbinds == grille->nbindsmalloc) {
		grille->nbindsmalloc += 100;
		grille->inds = (C_IND *)realloc((void *)grille->inds, grille->nbindsmalloc * sizeof(C_IND));
	}

	grille->inds[grille->nbinds].i1 = checked_range_cast<unsigned short>(i0);
	grille->inds[grille->nbinds].i2 = checked_range_cast<unsigned short>(i1);
	grille->inds[grille->nbinds++].i3 = checked_range_cast<unsigned short>(i2);
	grille->mats[matIdx].nbind += 3;

}

void AddQuadUVs(CinematicGrid * grille, int depcx, int depcy, int tcx, int tcy, int bitmapposx, int bitmapposy, int bitmapwx, int bitmapwy, Texture2D* tex)
{
	int matIdx = AddMaterial(grille, tex);
	grille->mats[matIdx].bitmapdep.x = bitmapposx;
	grille->mats[matIdx].bitmapdep.y = bitmapposy;
	grille->mats[matIdx].bitmap.x = bitmapwx;
	grille->mats[matIdx].bitmap.y = bitmapwy;
	grille->mats[matIdx].nbvertexs = (tcx + 1) * (tcy + 1);

	if((grille->nbuvs + (4 *(tcx * tcy))) > grille->nbuvsmalloc) {
		grille->nbuvsmalloc += 4 * (tcx * tcy);
		grille->uvs = (C_UV *)realloc((void *)grille->uvs, grille->nbuvsmalloc * sizeof(C_UV));
	}

	float v = 0.f;
	float du = 0.999999f / (float)tcx;
	float dv = 0.999999f / (float)tcy;
	tcy++;
	tcx++;
	int tcyy = tcy;
	int depcyy = depcy;

	while(tcyy--) {
		float u = 0.f;
		int depcxx = depcx;
		int tcxx = tcx;

		while(tcxx--) {
			int i0, i1, i2, i3;
			GetIndNumCube(grille, depcxx, depcyy, &i0, &i1, &i2, &i3);

			//uvs
			grille->uvs[grille->nbuvs].indvertex = i0;
			grille->uvs[grille->nbuvs].uv.x = u;
			grille->uvs[grille->nbuvs++].uv.y = v;
			depcxx++;
			u += du;
		}

		depcyy++;
		v += dv;
	}

	tcx--;
	tcy--;

	while(tcy--) {
		int depcxx = depcx;
		int tcxx = tcx;

		while(tcxx--) {
			int i0, i1, i2, i3;
			GetIndNumCube(grille, depcxx, depcy, &i0, &i1, &i2, &i3);

			AddPoly(grille, matIdx, i0, i1, i2);
			AddPoly(grille, matIdx, i1, i2, i3);
			depcxx++;
		}

		depcy++;
	}
}

CinematicBitmap* CreateCinematicBitmap(const res::path & path, int scale) {
	
	string name = path.basename();
	if(name.empty()) {
		return 0;
	}
	
	CinematicBitmap	* bi = new CinematicBitmap();
	if(!bi)
		return 0;

	LogDebug("loading cinematic texture " << path);

	size_t size = 0;
	
	res::path filename = path;
	filename.set_ext("bmp");
	char * data = resources->readAlloc(filename, size);
	if(!data) {
		filename.set_ext("tga");
		data = resources->readAlloc(filename, size);
	}

	if(!data) {
		LogError << path << " not found";
		delete bi;
		return 0;
	}

	Image cinematicImage;
	cinematicImage.LoadFromMemory(data, size);

	free(data);

	unsigned int width = cinematicImage.GetWidth();
	unsigned int height = cinematicImage.GetHeight();
	int nbx = width / MaxW;
	int nby = height / MaxH;

	if(width % MaxW)
		nbx++;

	if(height % MaxH)
		nby++;

	bi->m_size.x = width;
	bi->m_size.y = height;

	bi->nbx = nbx;
	bi->nby = nby;

	AllocGrille(&bi->grid, nbx, nby, (float)bi->m_size.x, (float)bi->m_size.y, (float)((bi->m_size.x > MaxW) ? MaxW : bi->m_size.x), (float)((bi->m_size.y > MaxH) ? MaxH : bi->m_size.y), scale);

	int num = 0;
	int h = bi->m_size.y;

	while(nby) {

		int h2;

		if((h - MaxH) < 0)
			h2 = h;
		else
			h2 = MaxH;

		int w = bi->m_size.x;
		int nbxx = nbx;

		while(nbxx) {
			int w2 = (w - MaxW) < 0 ? w : MaxW;

			Texture2D* tex = GRenderer->CreateTexture2D();
			tex->Init(w2, h2, cinematicImage.GetFormat());
			tex->GetImage().Copy(cinematicImage, 0, 0, bi->m_size.x - w, bi->m_size.y - h, w2, h2);
			tex->Upload();

			AddQuadUVs(&bi->grid, (bi->nbx - nbxx) * scale, (bi->nby - nby) * scale, scale, scale, bi->m_size.x - w, bi->m_size.y - h, w2, h2, tex);

			w -= MaxW;

			num++;
			nbxx--;
		}

		h -= MaxH;
		nby--;
	}

	bi->grid.echelle = scale;

	ReajustUV(bi);

	return bi;
}

static void ReajustUV(CinematicBitmap* cb) {

	C_UV* uvs = cb->grid.uvs;

	for(std::vector<C_INDEXED>::iterator mat = cb->grid.mats.begin(); mat != cb->grid.mats.end(); ++mat)
	{
		Texture2D* tex = mat->tex;

		if(!tex)
			return;

		int w, h;

		w = tex->getStoredSize().x;
		h = tex->getStoredSize().y;

		if((w != (int)mat->bitmap.x) || (h != (int)mat->bitmap.y)) {
			float dx = (.999999f * (float)mat->bitmap.x) / ((float)w);
			float dy = (.999999f * (float)mat->bitmap.y) / ((float)h);

			int nb2 = mat->nbvertexs;

			while(nb2--) {
				uvs->uv.x *= dx;
				uvs->uv.y *= dy;
				uvs++;
			}
		} else {
			uvs += mat->nbvertexs;
		}
	}
}
