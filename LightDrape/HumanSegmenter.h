#pragma once
#include "Meshsegmenter.h"
#include "Segment.h"
class HumanSegmenter :
	public MeshSegmenter
{
public:
	HumanSegmenter(WatertightMesh_ mesh) : MeshSegmenter(mesh){
		/* C++构造函数中调用虚函数时是不会调用子类函数的，所以分开调用init.(Java是可以的） */
		initHumanSegmenter(mesh);
	}
	HumanSegmenter(void){}

	void init(WatertightMesh_ mesh){		
		MeshSegmenter::init(mesh);
		initHumanSegmenter(mesh);
	}

	~HumanSegmenter(void);
private:
	Region_ mHead;
	Region_ mTorso;
	Region_ mLeftHand;
	Region_ mRightHand;
	Region_ mLeftLeg;
	Region_ mRightLeg;
	size_t mTorsoSkeNode, mLeftHandSkeNode, mRightHandSkeNode
		, mLeftLegSkeNode, mRightLegSkeNode;
	WatertightMesh_ mMesh;
	Skeleton_ mMeshSkeleton;
protected:
	/* Override */
	void onDifferentLevelSet(size_t seq, LevelSet& levelSet){
		if(seq == 1){
			handleHead(levelSet);
		}
		else if(seq == 2){
			handleTorso(levelSet);
		}
		else if(seq == 3){
			handleLegs(levelSet);
		}
	}

	/* Override */
	void onFinishSegment(){
		Segment_ seg = mMesh->getSegment();
		std::vector< std::pair<size_t, Region_> >& regions = seg->getRegionsRaw();
		for(std::vector< std::pair<size_t, Region_> >::iterator it = regions.begin();
			it != regions.end(); it++){
				it->second->expand();
		}
	}

	/* Override */
	Segment_ createSegment(){
		Segment_ seg = smartNew(Segment);
		mHead = std::make_shared<Region>(mMeshSkeleton);
		mTorso = std::make_shared<Region>(mMeshSkeleton);
		mLeftHand = std::make_shared<Region>(mMeshSkeleton);
		mRightHand = std::make_shared<Region>(mMeshSkeleton);
		mLeftLeg = std::make_shared<Region>(mMeshSkeleton);
		mRightLeg = std::make_shared<Region>(mMeshSkeleton);
		seg->addRegionRaw(Segment::BODY_HEAD, mHead);
		seg->addRegionRaw(Segment::BODY_TORSE, mTorso);
		seg->addRegionRaw(Segment::BODY_LEFT_HAND, mLeftHand);
		seg->addRegionRaw(Segment::BODY_RIGHT_HAND, mRightHand);
		seg->addRegionRaw(Segment::BODY_LEFT_LEG, mLeftLeg);
		seg->addRegionRaw(Segment::BODY_RIGHT_LEG, mRightLeg);
		return seg;
	}

private:
	void initHumanSegmenter(WatertightMesh_ mesh){
		mTorsoSkeNode = mLeftHandSkeNode = mRightHandSkeNode
			= mLeftLegSkeNode = mRightLegSkeNode = -1;
		mMesh = mesh;
		mMeshSkeleton = mMesh->getSkeleton();
	}

	void handleHead(LevelSet& levelSet){
		if(levelSet.getCount() != 1){
			PRINTLN("Human Segment Error: incorrect categories of head");
			return ;
		}
		addToRegion(mHead, levelSet.getCircle(0));
	}


	void handleTorso(LevelSet& levelSet){
		if(levelSet.getCount() != 3){
			PRINTLN("Human Segment Error: incorrect categories of torso");
			return ;
		}
		if(mTorsoSkeNode == -1){
			/* compute mLeftHandSkeNode, mTorsoSkeNode, mRightHandSkeNode */
			typedef std::pair<LevelCircle_, double> CircleCenterPair;				
			LevelCircle_ lc0 = levelSet.getCircle(0);
			LevelCircle_ lc1 = levelSet.getCircle(1);
			LevelCircle_ lc2 = levelSet.getCircle(2);				
			CircleCenterPair ccp[3] = {
				std::make_pair(lc0, lc0->getCenterX(mMesh)),
				std::make_pair(lc1, lc1->getCenterX(mMesh)),
				std::make_pair(lc2, lc2->getCenterX(mMesh))};
			std::sort(ccp, ccp + 3, [](const CircleCenterPair& a, const CircleCenterPair& b){
				return a.second < b.second;
			});
			mLeftHandSkeNode = getCircleSkeletonNode(ccp[0].first);
			mTorsoSkeNode = getCircleSkeletonNode(ccp[1].first);
			mRightHandSkeNode = getCircleSkeletonNode(ccp[2].first);
			addToRegion(mLeftHand, ccp[0].first);
			addToRegion(mTorso, ccp[1].first);
			addToRegion(mRightHand, ccp[2].first);
		}
		else{			
			for(size_t i = 0; i < 3; i++){
				LevelCircle_ lc = levelSet.getCircle(i);
				size_t skeNode = getCircleSkeletonNode(lc);
				size_t disLH = mMeshSkeleton->nodeDis(skeNode, mLeftHandSkeNode);
				size_t disTS = mMeshSkeleton->nodeDis(skeNode, mTorsoSkeNode);
				size_t disRH = mMeshSkeleton->nodeDis(skeNode, mRightHandSkeNode);
				if(disLH <= disTS && disLH <= disRH){
					addToRegion(mLeftHand, lc);
					mLeftHandSkeNode = skeNode;					
				}
				else if(disTS <= disLH && disTS <= disRH){
					addToRegion(mTorso, lc);
					mTorsoSkeNode = skeNode;
				}
				else if(disRH <= disTS && disRH <= disLH){
					addToRegion(mRightHand, lc);
					mRightHandSkeNode = skeNode;
				}
// 				if(mMeshSkeleton->isNodeNeighbor(skeNode, mLeftHandSkeNode)){
// 					addToRegion(mLeftHand, lc);
// 					mLeftHandSkeNode = skeNode;
// 				}
// 				else if(mMeshSkeleton->isNodeNeighbor(skeNode, mTorsoSkeNode)){
// 					addToRegion(mTorso, lc);
// 					mTorsoSkeNode = skeNode;
// 				}
// 				else if(mMeshSkeleton->isNodeNeighbor(skeNode, mRightHandSkeNode)){
// 					addToRegion(mRightHand, lc);
// 					mRightHandSkeNode = skeNode;
// 				}
			}
		}
	}


	void handleLegs(LevelSet& levelSet){
		if(levelSet.getCount() != 2){
			PRINTLN("Human Segment Error: incorrect categories of legs");
			return ;
		}
		LevelCircle_ leg1 = levelSet.getCircle(0);
		LevelCircle_ leg2 = levelSet.getCircle(1);
		size_t skeNode1 = getCircleSkeletonNode(leg1);
		size_t skeNode2 = getCircleSkeletonNode(leg2);
		if(mLeftLegSkeNode == -1){
			double c1 = leg1->getCenterX(mMesh);
			double c2 = leg2->getCenterX(mMesh);
			if(c1 < c2){
				mLeftLegSkeNode = skeNode1;
				mRightLegSkeNode = skeNode2;
				addToRegion(mLeftLeg, leg1);
				addToRegion(mRightLeg, leg2);
			}
			else{
				mLeftLegSkeNode = skeNode2;
				mRightLegSkeNode = skeNode1;				
				addToRegion(mLeftLeg, leg2);
				addToRegion(mRightLeg, leg1);
			}
		}
		else{
			size_t dis1 = mMeshSkeleton->nodeDis(skeNode1, mLeftLegSkeNode);
			size_t dis2 = mMeshSkeleton->nodeDis(skeNode1, mRightLegSkeNode);
			if(dis1 < dis2){
				addToRegion(mLeftLeg, leg1);
				addToRegion(mRightLeg, leg2);
				mLeftLegSkeNode = skeNode1;
				mRightLegSkeNode = skeNode2;
			}
			else{
				addToRegion(mLeftLeg, leg2);
				addToRegion(mRightLeg, leg1);
				mLeftLegSkeNode = skeNode2;
				mRightLegSkeNode = skeNode1;
			}
		}
	}
};

