#include "pch.h"
#include "NaviCell.h"
#include "NaviMesh.h"

int NaviCell::GetCellIndex(const NaviMesh* const curNaviMesh) const noexcept
{
    return curNaviMesh->GetNaviCellIndex(this);
}

const NaviCell* NaviCell::GetNeighbourhood(const NaviMesh* const curNaviMesh,const int idx) const noexcept
{
    const int cell_idx = m_neighbourhoodsIdx[idx];
    return -1 == cell_idx ? nullptr : curNaviMesh->GetNaviCell(cell_idx);
}

