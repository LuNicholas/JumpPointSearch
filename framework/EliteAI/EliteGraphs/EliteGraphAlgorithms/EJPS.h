#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class JPS
	{
	public:
		JPS(GridGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		
		struct JPSNode
		{
			T_NodeType* pNode = nullptr;
			T_NodeType* pParentNode = nullptr;


			bool operator==(const JPSNode& other) const
			{
				return pNode == other.pNode
					&& pParentNode == other.pParentNode;
			};

		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		std::vector<T_NodeType*> GetSuccessors(JPSNode* currentNode, T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		T_NodeType* Jump(T_NodeType* Parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		std::vector<T_NodeType*> GetNeighbours(JPSNode* node) const;

		bool IsNodeBlocked(float col, float row)const;

		GridGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	JPS<T_NodeType, T_ConnectionType>::JPS(GridGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> Elite::JPS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<JPSNode*> openList;
		std::vector<JPSNode*> closedList;
		std::vector<JPSNode*> testList;

		JPSNode* startNode = new JPSNode{};
		startNode->pNode = pStartNode;
		startNode->pParentNode = pStartNode;
		openList.push_back(startNode);

		while (!openList.empty())
		{
			//for (JPSNode* currentNode : openList)
			for(int idx = 0;idx < openList.size(); idx++ )
			{
				JPSNode* currentNode = openList.at(idx);


				//goalNode found
				//if (currentNode->pNode == pGoalNode)
				//{
				//	std::vector<T_NodeType*> testPath;
				//	for (JPSNode* newNode : closedList)
				//	{
				//		testPath.push_back(newNode->pNode);
				//	}
				//	return testPath;
				//	break;
				//}

				std::vector<T_NodeType*> successorNodes = GetSuccessors(currentNode, pStartNode, pGoalNode);
				
				closedList.push_back(currentNode);
				

				for (T_NodeType* successedNode : successorNodes)
				{
					JPSNode* pSuccessor = new JPSNode{};
					pSuccessor->pNode = successedNode;
					pSuccessor->pParentNode = currentNode->pNode;

					testList.push_back(pSuccessor);
				}
				openList.erase(std::remove(openList.begin(), openList.end(), currentNode));
			}

			for (JPSNode* newNode : testList)
			{
				openList.push_back(newNode);
			}
			testList.clear();

		}


		std::vector<T_NodeType*> testPath;
		for (JPSNode* newNode : closedList)
		{
			testPath.push_back(newNode->pNode);
		}

		delete startNode;
		return testPath;
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> Elite::JPS<T_NodeType, T_ConnectionType>::GetSuccessors(JPSNode* currentNode, T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		std::vector<T_NodeType*> successors;
		std::vector<T_NodeType*> neigbours;

		if (currentNode->pNode == pStartNode)
		{
			for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(pStartNode->GetIndex()))
			{
				neigbours.push_back(m_pGraph->GetNode(connection->GetTo()));
			}
		}
		else
		{
			neigbours = GetNeighbours(currentNode);
		}




		for (T_NodeType* neigbour : neigbours)
		{

			Elite::Vector2  neigbourPos = m_pGraph->GetNodePos(neigbour);
			Elite::Vector2  currentPos = m_pGraph->GetNodePos(currentNode->pNode->GetIndex());

			int directionX = Clamp<int>(int(neigbourPos.x - currentPos.x), -1, 1);
			int directionY = Clamp<int>(int(neigbourPos.y - currentPos.y), -1, 1);


			//Get JumpPoint
			T_NodeType* jumpPoint = Jump(currentNode->pNode, directionX, directionY, pStartNode, pEndNode);


			if (jumpPoint != nullptr)
				successors.push_back(jumpPoint);
		}
		return successors;
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> Elite::JPS<T_NodeType, T_ConnectionType>::GetNeighbours(JPSNode* node) const
	{

		Elite::Vector2 parentPos = m_pGraph->GetNodePos(node->pParentNode->GetIndex());
		Elite::Vector2 pos = m_pGraph->GetNodePos(node->pNode->GetIndex());

		std::vector<T_NodeType*> neighbours;


		int directionX = Clamp<int>(int(pos.x - parentPos.x), -1, 1);
		int directionY = Clamp<int>(int(pos.y - parentPos.y), -1, 1);




		if (directionY == 0)//if direction Y is 0 only horizontal movement is possible
		{
			if (!IsNodeBlocked(pos.x + directionX, pos.y))//if the next node of this node is not blocked
			{
				neighbours.push_back(m_pGraph->GetNode(int(pos.x + directionX), int(pos.y)));//put next node as neighbour

				//CHECKING FOR FORCED NEIGHBOUR
				if (IsNodeBlocked(pos.x, pos.y - 1))//if the node below is blocked
					if(!IsNodeBlocked(pos.x + directionX, pos.y - 1))
						neighbours.push_back(m_pGraph->GetNode(int(pos.x + directionX), int(pos.y - 1)));//push back a forced neighbour

				//CHECKING FOR FORCED NEIGHBOUR
				if (IsNodeBlocked(pos.x, pos.y + 1))//if the node above is blocked
					if (!IsNodeBlocked(pos.x + directionX, pos.y + 1))
						neighbours.push_back(m_pGraph->GetNode(int(pos.x + directionX), int(pos.y + 1)));//push back a forced neighbour
			}

			return neighbours;
		}


		if (directionX == 0)//if direction X is 0 only vertical movement is possible
		{
			if (!IsNodeBlocked(pos.x, pos.y + directionY))//if the next node of this node is not blocked
			{
				neighbours.push_back(m_pGraph->GetNode(int(pos.x), int(pos.y + directionY)));//put next node as neighbour

				//CHECKING FOR FORCED NEIGHBOUR
				if (IsNodeBlocked(pos.x + 1, pos.y))//if the node to the right is blocked
					if (!IsNodeBlocked(pos.x + 1, pos.y + directionY))
						neighbours.push_back(m_pGraph->GetNode(int(pos.x + 1), int(pos.y + directionY)));//push back a forced neighbour

				//CHECKING FOR FORCED NEIGHBOUR
				if (IsNodeBlocked(pos.x - 1, pos.y))//if the node to the left is blocked
					if (!IsNodeBlocked(pos.x - 1, pos.y + directionY))
						neighbours.push_back(m_pGraph->GetNode(int(pos.x - 1),int(pos.y + directionY)));//push back a forced neighbour
			}
			return neighbours;
		}


		//diagonal movement
		if (!IsNodeBlocked(pos.x + directionX, pos.y + directionY))
			neighbours.push_back(m_pGraph->GetNode(int(pos.x + directionX), int(pos.y + directionY)));

		if (!IsNodeBlocked(pos.x + directionX, pos.y))
			neighbours.push_back(m_pGraph->GetNode(int(pos.x + directionX), int(pos.y)));

		if(!IsNodeBlocked(pos.x, pos.y + directionY))
			neighbours.push_back(m_pGraph->GetNode(int(pos.x), int(pos.y + directionY)));


		//CHECKING FOR FORCED NEIGHBOUR
		if (IsNodeBlocked(parentPos.x + directionX, parentPos.y))
		{
			if (!IsNodeBlocked(pos.x + directionX, pos.y - directionY))
				neighbours.push_back(m_pGraph->GetNode(int(pos.x + directionX), int(pos.y - directionY)));
		}
		if (IsNodeBlocked(parentPos.x, parentPos.y + directionY))
		{
			if (!IsNodeBlocked(pos.x - directionX, pos.y + directionY))
				neighbours.push_back(m_pGraph->GetNode(int(pos.x - directionX), int(pos.y + directionY)));
		}
		return neighbours;

	}

	template <class T_NodeType, class T_ConnectionType>
	T_NodeType* Elite::JPS<T_NodeType, T_ConnectionType>::Jump(T_NodeType* parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		//position of new node
		int connections{ 0 };
		Elite::Vector2 nextPos{ m_pGraph->GetNodePos(parent) + Elite::Vector2{float(horizontal), float(vertical)} };

		if (IsNodeBlocked(nextPos.x, nextPos.y))//node is either out of bound or blocked(water)
		{
			return nullptr;
		}


		//the new node found is the endNode
		if (nextPos == m_pGraph->GetNodePos(pEndNode))
			return pEndNode;

		
		
		if (horizontal != 0 && vertical != 0)//current search is a diagonal search
		{
			//check horizontal and vertical for 
			
			if (Jump(m_pGraph->GetNode(int(nextPos.x), int(nextPos.y)), horizontal, 0, pStartNode, pEndNode) != nullptr ||
				Jump(m_pGraph->GetNode(int(nextPos.x), int(nextPos.y)), 0, vertical, pStartNode, pEndNode) != nullptr)
			{
				return m_pGraph->GetNode(int(nextPos.x), int(nextPos.y));
			}


		}
		else if(horizontal != 0)//horizontal search
		{
			Elite::Vector2 currentPos{ m_pGraph->GetNodePos(parent) };

			if (IsNodeBlocked(nextPos.x, nextPos.y + 1))//check if node above is blocked
			{
				if (!IsNodeBlocked(nextPos.x + horizontal, nextPos.y + 1))
				{
					return m_pGraph->GetNode(int(nextPos.x), int(nextPos.y));
				}
			}

			if (IsNodeBlocked(nextPos.x, nextPos.y - 1))//check if node above is blocked
			{
				if (!IsNodeBlocked(nextPos.x + horizontal, nextPos.y - 1))
				{
					return m_pGraph->GetNode(int(nextPos.x), int(nextPos.y));
				}
			}


		}
		else//vertical search
		{
			Elite::Vector2 currentPos{ m_pGraph->GetNodePos(parent) };

			if (IsNodeBlocked(nextPos.x + 1, nextPos.y))//check if node right of the next node is free 
			{
				if (!IsNodeBlocked(nextPos.x + 1, nextPos.y + vertical))
				{
					return m_pGraph->GetNode(int(nextPos.x), int(nextPos.y));
				}
			}

			if (IsNodeBlocked(nextPos.x - 1, nextPos.y))//check if node left of the next node is free 
			{
				if (!IsNodeBlocked(nextPos.x - 1, nextPos.y + vertical))
				{
					return m_pGraph->GetNode(int(nextPos.x), int(nextPos.y));
				}
			}

		}

		T_NodeType* nextNode{m_pGraph->GetNode(int(nextPos.x), int(nextPos.y))};
		Jump(nextNode, horizontal, vertical, pStartNode, pEndNode);


	}



	template <class T_NodeType, class T_ConnectionType>
	bool Elite::JPS<T_NodeType, T_ConnectionType>::IsNodeBlocked(float col, float row) const
	{
		if (!m_pGraph->IsWithinBounds(int(col), int(row)))//check if the node is in the grid boundaries;
			return true;

		int nodeIdx = m_pGraph->GetIndex(int(col), int(row));//get node idx from position
		if (m_pGraph->GetNode(nodeIdx)->GetTerrainType() == TerrainType::Water)//check if node isnt blocked
			return true;

		return false;
	}


	template <class T_NodeType, class T_ConnectionType>
	float Elite::JPS<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}



}