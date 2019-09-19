#include "ndpch.h"
#include "TreeGen.h"
#include "world/block/block_datas.h"
#include "world/block/basic_blocks.h"
#include "world/World.h"

//chance of true determined by 1/x
#define diceRoll(x)\
	((std::rand()%(x))==0)

//change of true in percent
#define bigDiceRoll(x)\
	((std::rand()%100)<(x))

bool setTreeBlock(World& w, int x, int y, int corner, int width, int height, bool overwrite = false)
{
	//check if there is space
	if (!overwrite)
		for (int xx = 0; xx < width; ++xx)
			for (int yy = 0; yy < height; ++yy)
				if (!w.isAir(x + xx, y + yy))
					return false;


	for (int xx = 0; xx < width; ++xx)
	{
		for (int yy = 0; yy < height; ++yy)
		{
			BlockStruct s = {};
			s.block_id = BLOCK_TREE;
			s.block_corner = corner;
			s.block_metadata = half_int(xx, yy);
			w.setBlockWithNotify(x + xx, y + yy, s);
		}
	}
	return true;
}


bool TreeGen::buildTree(World& w, int x, int y,bool useTrueRandom)
{
	using namespace BlockTreeScope;

	if(useTrueRandom)
		std::srand((x + 10)*(y - 10) + (y>1) * 123456 - x * 456789);
	
	constexpr int minHeight = 15;
	int treeHeight = std::rand() % 10 + minHeight;
	int minnerTrunkHeight = std::rand() % 3 + 1;
	int minniTrunkHeight = std::rand() % 3 + 1;

	//check if enough space for roots
	for (int xx = -1; xx < 3; ++xx)
	{
		if (!w.isAir(xx + x, y) && xx != 0)
			return false;
	}
	bool shouldBreak = false;
	//check for at least some space for trunk
	for (int yy = 1; yy < treeHeight; ++yy)
	{
		for (int xx = -1; xx < 3; ++xx)
		{
			if (!w.isAir(x + xx, y + yy))
			{
				treeHeight = yy;
				minnerTrunkHeight = std::rand() % 2 + 1;
				minniTrunkHeight = std::rand() % 2 + 1;
				shouldBreak = true;
				break;
			}
		}
		if (shouldBreak)
			break;
	}
	if (treeHeight < minHeight) //at least n blocks high
		return false;

	//base root
	w.beginBlockSet();
	setTreeBlock(w, x - 1, y, RootL, 1, 1, true);
	setTreeBlock(w, x + 2, y, RootR, 1, 1, true);
	setTreeBlock(w, x, y, fullTrunk2W, 2, 1, true);
	int yLevel = treeHeight;

	//appendix of blob and bigblob0
	yLevel -= 2;
	setTreeBlock(w, x, y + yLevel, blob, 2, 2);
	yLevel -= 2;
	setTreeBlock(w, x - 1, y + yLevel, bigBlob1, 4, 2);

	bool hasBlob = false;
	for (int i = minniTrunkHeight - 1; i >= 0; --i)
	{
		if (bigDiceRoll(50) && i > 0)
		{
			yLevel -= 2;
			setTreeBlock(w, x - 1, y + yLevel, bigBlob1, 4, 2);
			hasBlob = true;
		}
		else
		{
			yLevel -= 1;
			setTreeBlock(w, x, y + yLevel, thinTrunkL, 1, 1);
			setTreeBlock(w, x + 1, y + yLevel, thinTrunkR, 1, 1);
			hasBlob = false;
		}
	}
	if (!hasBlob)
	{
		yLevel -= 2;
		setTreeBlock(w, x - 1, y + yLevel, bigBlob1, 4, 2);
	}
	for (int i = minnerTrunkHeight - 1; i >= 0; --i)
	{
		if (bigDiceRoll(50) && i > 0)
		{
			yLevel -= 2;
			setTreeBlock(w, x - 2, y + yLevel, bigBlob0, 6, 2);
		}
		else
		{
			yLevel -= 1;
			setTreeBlock(w, x, y + yLevel, trunkL, 1, 1);
			setTreeBlock(w, x + 1, y + yLevel, trunkR, 1, 1);
		}
	}
	//add trunk and branches

	int yLevelOld = yLevel;
	int lastBigBranch = 1000;
	bool hasOneBranch = false;
	//left side
	while (yLevel > 0)
	{
		if (yLevel > 2 && (yLevelOld - yLevel) > 2)
		{
			//left branch
			if (bigDiceRoll(hasOneBranch?50:90))
			{
				hasOneBranch = true;
				setTreeBlock(w, x, y + yLevel, trunkBranchL, 1, 1);

				switch (std::rand() % 5)
				{
				case 0:
				MakeSmallBranch:
					setTreeBlock(w, x - 1, y + yLevel, dryBranchL, 1, 1);

					break;

				case 1:
					if (lastBigBranch - yLevel <= 2)
						goto MakeSmallBranch;
					setTreeBlock(w, x - 1, y + yLevel, dryBranchL, 1, 1);
					setTreeBlock(w, x - 2, y + yLevel + 1, dryBlobL, 2, 1);
					lastBigBranch = yLevel;

					break;
				case 2:
				case 3:
				case 4:
					if (lastBigBranch - yLevel <= 2)
						goto MakeSmallBranch;
					setTreeBlock(w, x - 1, y + yLevel, branchL, 1, 1);
					setTreeBlock(w, x - 2, y + yLevel + 1, smallBlobL2W, 2, 1);
					lastBigBranch = yLevel;
					break;
				}
				--yLevel;
				continue;
			}
		}
		setTreeBlock(w, x, y + yLevel, trunkL, 1, 1);
		--yLevel;
	}

	yLevel = yLevelOld;
	lastBigBranch = 1000;
	hasOneBranch = false;


	//right side
	while (yLevel > 0)
	{
		if (yLevel > 2 && (yLevelOld - yLevel) > 2)
		{
			//right branch
			if (bigDiceRoll(hasOneBranch ? 50 : 90))
			{
				hasOneBranch = true;
				setTreeBlock(w, x + 1, y + yLevel, trunkBranchR, 1, 1);

				switch (std::rand() % 5)
				{
				case 0:
				MakeSmallBranchR:
					setTreeBlock(w, x + 2, y + yLevel, dryBranchR, 1, 1);

					break;

				case 1:
					if (lastBigBranch - yLevel <= 2)
						goto MakeSmallBranchR;
					setTreeBlock(w, x + 2, y + yLevel, dryBranchR, 1, 1);
					setTreeBlock(w, x + 2, y + yLevel + 1, dryBlobR, 2, 1);
					lastBigBranch = yLevel;

					break;
				case 2:
				case 3:
				case 4:
					if (lastBigBranch - yLevel <= 2)
						goto MakeSmallBranchR;
					setTreeBlock(w, x + 2, y + yLevel, branchR, 1, 1);
					setTreeBlock(w, x + 2, y + yLevel + 1, smallBlobR2W, 2, 1);
					lastBigBranch = yLevel;
					break;
				}
				--yLevel;
				continue;
			}
		}
		setTreeBlock(w, x + 1, y + yLevel, trunkR, 1, 1);
		--yLevel;
	}
	/*while (yLevel > 1)
	{
		if (((yLevel & 1) == 0) && yLevel > 2)
		{
			if (diceRoll(3))
			{
				//right branch
				if (bigDiceRoll(60))
				{
					setTreeBlock(w, x + 1, y + yLevel, trunkBranchR, 1, 1);


					switch (std::rand() % 4)
					{
					case 0:
						setTreeBlock(w, x + 2, y + yLevel, dryBranchR, 1, 1);
						setTreeBlock(w, x + 2, y + yLevel + 1, dryBlobR, 2, 1);
						break;
					case 1:
					case 2:
						setTreeBlock(w, x + 2, y + yLevel, dryBranchR, 1, 1);
						break;
					case 3:
						setTreeBlock(w, x + 2, y + yLevel, branchR, 1, 1);
						setTreeBlock(w, x + 2, y + yLevel + 1, smallBlobR2W, 2, 1);
						break;
					}
				}
				else
					setTreeBlock(w, x + 1, y + yLevel, trunkR, 1, 1);
				--yLevel;
				continue;
			}
		}
		setTreeBlock(w, x, y + yLevel, trunkL, 1, 1);
		setTreeBlock(w, x + 1, y + yLevel, trunkR, 1, 1);

		--yLevel;
	}*/

	w.flushBlockSet();
	return true;
}
