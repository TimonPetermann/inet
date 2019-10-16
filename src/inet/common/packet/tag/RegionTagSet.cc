//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/tag/RegionTagSet.h"

namespace inet {

RegionTagSet::RegionTagSet() :
    regionTags(nullptr)
{
}

RegionTagSet::RegionTagSet(const RegionTagSet& other) :
    regionTags(nullptr)
{
    operator=(other);
}

RegionTagSet::RegionTagSet(RegionTagSet&& other) :
    regionTags(other.regionTags)
{
    other.regionTags = nullptr;
}

RegionTagSet& RegionTagSet::operator=(const RegionTagSet& other)
{
    if (this != &other) {
        clearAllTags();
        if (other.regionTags == nullptr)
            regionTags = nullptr;
        else {
            ensureAllocated();
            for (auto& regionTag : *other.regionTags)
                addTag(regionTag.getOffset(), regionTag.getLength(), regionTag.getTag()->dup());
        }
    }
    return *this;
}

RegionTagSet& RegionTagSet::operator=(RegionTagSet&& other)
{
    if (this != &other) {
        clearAllTags();
        regionTags = other.regionTags;
        other.regionTags = nullptr;
    }
    return *this;
}

RegionTagSet::~RegionTagSet()
{
    clearAllTags();
}

void RegionTagSet::ensureAllocated()
{
    if (regionTags == nullptr) {
        regionTags = new std::vector<RegionTag<cObject>>();
        regionTags->reserve(16);
    }
}

void RegionTagSet::addTag(b offset, b length, cObject *tag)
{
    ensureAllocated();
    regionTags->push_back(RegionTag<cObject>(offset, length, tag));
    std::sort(regionTags->begin(), regionTags->end());
    if (tag->isOwnedObject())
        take(static_cast<cOwnedObject *>(tag));
}

void RegionTagSet::mapAllTags(b offset, b length, std::function<void (b, b, cObject *)> f) const
{
    if (regionTags != nullptr) {
        b startOffset = offset;
        b endOffset = offset + length;
        for (auto& regionTag : *regionTags) {
            if (endOffset <= regionTag.getStartOffset() || regionTag.getEndOffset() <= startOffset)
                // no intersection
                continue;
            else if (startOffset <= regionTag.getStartOffset() && regionTag.getEndOffset() <= endOffset)
                // totally covers region
                f(regionTag.getOffset(), regionTag.getLength(), regionTag.getTag());
            else if (regionTag.getStartOffset() < startOffset && endOffset < regionTag.getEndOffset())
                // splits region into two parts
                f(startOffset, endOffset - startOffset, regionTag.getTag());
            else if (regionTag.getEndOffset() <= endOffset)
                // cuts end of region
                f(startOffset, regionTag.getEndOffset() - startOffset, regionTag.getTag());
            else if (startOffset <= regionTag.getStartOffset())
                // cuts beginning of region
                f(regionTag.getStartOffset(), endOffset - regionTag.getStartOffset(), regionTag.getTag());
            else
                ASSERT(false);
        }
    }
}

std::vector<RegionTagSet::RegionTag<cObject>> RegionTagSet::getAllTags(b offset, b length) const
{
    std::vector<RegionTagSet::RegionTag<cObject>> result;
    mapAllTags(offset, length, [&] (b o, b l, const cObject *t) {
        result.push_back(RegionTag<cObject>(o, l, t->dup()));
    });
    return result;
}

cObject *RegionTagSet::removeTag(int index)
{
    auto tag = (*regionTags)[index].getTag();
    if (tag->isOwnedObject())
        drop(static_cast<cOwnedObject *>(tag));
    regionTags->erase(regionTags->begin() + index);
    return tag;
}

int RegionTagSet::getTagIndex(const std::type_info& typeInfo, b offset, b length) const
{
    if (regionTags == nullptr)
        return -1;
    else {
        int numRegionTags = regionTags->size();
        b getStartOffset = offset;
        b getEndOffset = offset + length;
        for (int index = 0; index < numRegionTags; index++) {
            auto& regionTag = (*regionTags)[index];
            const cObject *tag = regionTag.getTag();
            if (typeInfo != typeid(*tag) || getEndOffset <= regionTag.getStartOffset() || regionTag.getEndOffset() <= getStartOffset)
                continue;
            else if (regionTag.getOffset() == offset && regionTag.getLength() == length)
                return index;
            else
                throw cRuntimeError("Overlapping tag is present");
        }
        return -1;
    }
}

void RegionTagSet::clearTags(b offset, b length)
{
    if (regionTags != nullptr) {
        b clearStartOffset = offset;
        b clearEndOffset = offset + length;
        for (auto it = regionTags->begin(); it != regionTags->end(); it++) {
            auto& regionTag = *it;
            if (clearEndOffset <= regionTag.getStartOffset() || regionTag.getEndOffset() <= clearStartOffset)
                // no intersection
                continue;
            else if (clearStartOffset <= regionTag.getStartOffset() && regionTag.getEndOffset() <= clearEndOffset) {
                // clear totally covers region
                auto tag = regionTag.getTag();
                if (tag->isOwnedObject())
                    drop(static_cast<cOwnedObject *>(tag));
                regionTags->erase(it--);
            }
            else if (regionTag.getStartOffset() < clearStartOffset && clearEndOffset < regionTag.getEndOffset()) {
                // clear splits region into two parts
                auto tag = regionTag.getTag()->dup();
                RegionTag<cObject> previousRegion(regionTag.getStartOffset(), clearStartOffset - regionTag.getStartOffset(), tag);
                regionTags->insert(it, previousRegion);
                if (tag->isOwnedObject())
                    take(static_cast<cOwnedObject *>(tag));
                regionTag.setLength(regionTag.getEndOffset() - clearEndOffset);
                regionTag.setOffset(clearEndOffset);
            }
            else if (regionTag.getEndOffset() <= clearEndOffset) {
                // clear cuts end of region
                regionTag.setLength(clearStartOffset - regionTag.getStartOffset());
            }
            else if (clearStartOffset <= regionTag.getStartOffset()) {
                // clear cuts beginning of region
                regionTag.setLength(regionTag.getEndOffset() - clearEndOffset);
                regionTag.setOffset(clearEndOffset);
            }
            else
                ASSERT(false);
        }
    }
}

void RegionTagSet::clearAllTags()
{
    if (regionTags != nullptr) {
        int numTags = regionTags->size();
        for (int index = 0; index < numTags; index++) {
            cObject *tag = (*regionTags)[index].getTag();
            if (tag->isOwnedObject())
                drop(static_cast<cOwnedObject *>(tag));
        }
        delete regionTags;
        regionTags = nullptr;
    }
}

void RegionTagSet::copyTags(const RegionTagSet& source, b sourceOffset, b offset, b length)
{
    clearTags(offset, length);
    ensureAllocated();
    auto shift = offset - sourceOffset;
    auto regionTags = source.getAllTags(sourceOffset, length);
    for (auto& regionTag : regionTags)
        addTag(regionTag.getOffset() + shift, regionTag.getLength(), regionTag.getTag()->dup());
}

void RegionTagSet::moveTags(b shift)
{
    if (regionTags != nullptr)
        for (auto& regionTag : *regionTags)
            regionTag.setOffset(regionTag.getOffset() + shift);
}

} // namespace

