//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/geometry/common/Rotation.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/OSGScene.h"
#include "inet/common/OSGUtils.h"
#include "inet/visualizer/physicallayer/TracingObstacleLossOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Geode>
#include <osg/LineWidth>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(TracingObstacleLossOsgVisualizer);

#ifdef WITH_OSG

void TracingObstacleLossOsgVisualizer::initialize(int stage)
{
    TracingObstacleLossVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        obstacleLossNode = new osg::Group();
        auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
        scene->addChild(obstacleLossNode);
    }
}

void TracingObstacleLossOsgVisualizer::refreshDisplay() const
{
    TracingObstacleLossVisualizerBase::refreshDisplay();
    // TODO: switch to osg canvas when API is extended
    visualizerTargetModule->getCanvas()->setAnimationSpeed(obstacleLossVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const TracingObstacleLossVisualizerBase::ObstacleLossVisualization *TracingObstacleLossOsgVisualizer::createObstacleLossVisualization(const IPhysicalObject *object, const Coord& intersection1, const Coord& intersection2, const Coord& normal1, const Coord& normal2) const
{
    const Rotation rotation(object->getOrientation());
    const Coord& position = object->getPosition();
    const Coord rotatedIntersection1 = rotation.rotateVectorClockwise(intersection1);
    const Coord rotatedIntersection2 = rotation.rotateVectorClockwise(intersection2);
    double intersectionDistance = intersection2.distance(intersection1);
    auto group = new osg::Group();
    if (displayIntersections) {
        auto geometry = inet::osg::createLineGeometry(rotatedIntersection1 + position, rotatedIntersection2 + position);
        auto geode = new osg::Geode();
        geode->addDrawable(geometry);
        geode->setStateSet(inet::osg::createLineStateSet(intersectionLineColor, intersectionLineStyle, intersectionLineWidth));
        group->addChild(geode);
    }
    if (displayFaceNormalVectors) {
        Coord normalVisualization1 = normal1 / normal1.length() * intersectionDistance / 10;
        Coord normalVisualization2 = normal2 / normal2.length() * intersectionDistance / 10;
        auto geometry1 = inet::osg::createLineGeometry(rotatedIntersection1 + position, rotatedIntersection1 + position + rotation.rotateVectorClockwise(normalVisualization1));
        auto geometry2 = inet::osg::createLineGeometry(rotatedIntersection2 + position, rotatedIntersection2 + position + rotation.rotateVectorClockwise(normalVisualization2));
        auto geode = new osg::Geode();
        geode->addDrawable(geometry1);
        geode->addDrawable(geometry2);
        geode->setStateSet(inet::osg::createLineStateSet(faceNormalLineColor, faceNormalLineStyle, faceNormalLineWidth));
        group->addChild(geode);
    }
    return new ObstacleLossOsgVisualization(group);
}

void TracingObstacleLossOsgVisualizer::addObstacleLossVisualization(const ObstacleLossVisualization* obstacleLossVisualization)
{
    TracingObstacleLossVisualizerBase::addObstacleLossVisualization(obstacleLossVisualization);
    auto obstacleLossOsgVisualization = static_cast<const ObstacleLossOsgVisualization *>(obstacleLossVisualization);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
    scene->addChild(obstacleLossOsgVisualization->node);
}

void TracingObstacleLossOsgVisualizer::removeObstacleLossVisualization(const ObstacleLossVisualization* obstacleLossVisualization)
{
    TracingObstacleLossVisualizerBase::removeObstacleLossVisualization(obstacleLossVisualization);
    auto obstacleLossOsgVisualization = static_cast<const ObstacleLossOsgVisualization *>(obstacleLossVisualization);
    auto node = obstacleLossOsgVisualization->node;
    node->getParent(0)->removeChild(node);
}

void TracingObstacleLossOsgVisualizer::setAlpha(const ObstacleLossVisualization *obstacleLossVisualization, double alpha) const
{
    auto obstacleLossOsgVisualization = static_cast<const ObstacleLossOsgVisualization *>(obstacleLossVisualization);
    auto node = obstacleLossOsgVisualization->node;
    for (int i = 0; i < node->getNumChildren(); i++) {
        auto material = static_cast<osg::Material *>(node->getChild(i)->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
        material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
    }
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

