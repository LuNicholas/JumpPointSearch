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
			JPSNode* pParentNode = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)



			bool operator==(const JPSNode& other) const
			{
				return pNode == other.pNode;
			};

			bool operator<(const JPSNode& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);
		std::vector<T_NodeType*> GetJumpPoints() const;

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		std::vector<T_NodeType*> GetSuccessors(const JPSNode& currentNode, T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		T_NodeType* Jump(T_NodeType* Parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		std::vector<T_NodeType*> GetNeighbours(const JPSNode& node) const;

		bool IsNodeBlocked(float col, float row)const;

		GridGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		std::vector<T_NodeType*> m_JumpPoints;
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
		std::vector<JPSNode> openList;
		std::vector<JPSNode> closedList;
		std::vector<T_NodeType*> path;
		m_JumpPoints.clear();

		JPSNode startNode;
		startNode.pNode = pStartNode;
		startNode.pParentNode = nullptr;
		startNode.costSoFar = 0;
		startNode.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);
		openList.push_back(startNode);


		while (!openList.empty())
		{
			JPSNode currentNode = *std::min_element(openList.begin(), openList.end());


			if (std::find(closedList.begin(), closedList.end(), currentNode) == closedList.end())
				closedList.push_back(currentNode);


			m_JumpPoints.push_back(currentNode.pNode);
			auto itNode = std::prev(closedList.end());
			openList.erase(std::remove(openList.begin(), openList.end(), currentNode));


			if (currentNode.pNode == pGoalNode)
			{
				path.clear();
				JPSNode newNode = currentNode;
				while (newNode.pParentNode != nullptr)
				{
					path.push_back(newNode.pNode);
					newNode = *newNode.pParentNode;
				}
				path.push_back(newNode.pNode);
				break;
			}


			std::vector<T_NodeType*> successorNodes = GetSuccessors(currentNode, pStartNode, pGoalNode);
			for (T_NodeType* successedNode : successorNodes)
			{
				auto openIt = std::find_if(openList.begin(), openList.end(), [successedNode](const JPSNode& lhs) {return lhs.pNode == successedNode; });
				auto closedIt = std::find_if(closedList.begin(), closedList.end(), [successedNode](const JPSNode& lhs) {return lhs.pNode == successedNode; });
				if (openIt == openList.end() && closedIt == closedList.end())
				{
					JPSNode pSuccessor;
					pSuccessor.pNode = successedNode;

					JPSNode* pParentNode = new JPSNode{};
					pParentNode->pNode = currentNode.pNode;
					pParentNode->pParentNode = currentNode.pParentNode;
					pParentNode->costSoFar = currentNode.costSoFar;
					pParentNode->estimatedTotalCost = currentNode.estimatedTotalCost;

					pSuccessor.pParentNode = pParentNode;
					pSuccessor.costSoFar = currentNode.costSoFar + m_pGraph->GetNodePos((*itNode).pNode).Distance(m_pGraph->GetNodePos(successedNode));
					pSuccessor.estimatedTotalCost = GetHeuristicCost(successedNode, pGoalNode) + pSuccessor.costSoFar;

					openList.push_back(pSuccessor);
				}
			}


		}

		for (JPSNode& node : closedList)
		{
			if (node.pParentNode)
			{
				delete node.pParentNode;
				node.pParentNode = nullptr;
			}
		}
		for (JPSNode& node : openList)
		{
			if (node.pParentNode)
			{
				delete node.pParentNode;
				node.pParentNode = nullptr;
			}
		}
		//return jumpPoints;

		std::reverse(path.begin(), path.end());
		return path;

	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> Elite::JPS<T_NodeType, T_ConnectionType>::GetSuccessors(const JPSNode& currentNode, T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		std::vector<T_NodeType*> successors;
		std::vector<T_NodeType*> neigbours;

		if (currentNode.pNode == pStartNode)
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
			Elite::Vector2  currentPos = m_pGraph->GetNodePos(currentNode.pNode->GetIndex());

			int directionX = Clamp<int>(int(neigbourPos.x - currentPos.x), -1, 1);
			int directionY = Clamp<int>(int(neigbourPos.y - currentPos.y), -1, 1);


			//Get JumpPoint
			T_NodeType* jumpPoint = Jump(currentNode.pNode, directionX, directionY, pStartNode, pEndNode);


			if (jumpPoint != nullptr)
			{
				successors.push_back(jumpPoint);
				if (jumpPoint == pEndNode)
					return successors;
			}
		}
		return successors;
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> Elite::JPS<T_NodeType, T_ConnectionType>::GetNeighbours(const JPSNode& node) const
	{

		Elite::Vector2 parentPos = m_pGraph->GetNodePos(node.pParentNode->pNode->GetIndex());
		Elite::Vector2 pos = m_pGraph->GetNodePos(node.pNode->GetIndex());

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
					if (!IsNodeBlocked(pos.x + directionX, pos.y - 1))
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
						neighbours.push_back(m_pGraph->GetNode(int(pos.x - 1), int(pos.y + directionY)));//push back a forced neighbour
			}
			return neighbours;
		}


		//diagonal movement
		bool canMoveDiagonal{ false };

		if (!IsNodeBlocked(pos.x + directionX, pos.y))
		{
			neighbours.push_back(m_pGraph->GetNode(int(pos.x + directionX), int(pos.y)));
			canMoveDiagonal = true;
		}

		if (!IsNodeBlocked(pos.x, pos.y + directionY))
		{ 
			neighbours.push_back(m_pGraph->GetNode(int(pos.x), int(pos.y + directionY)));
			canMoveDiagonal = true;
		}

		if (!IsNodeBlocked(pos.x + directionX, pos.y + directionY) && canMoveDiagonal)
			neighbours.push_back(m_pGraph->GetNode(int(pos.x + directionX), int(pos.y + directionY)));


		//CHECKING FOR FORCED NEIGHBOUR
		if (IsNodeBlocked(pos.x - directionX, pos.y))
		{
			if (!IsNodeBlocked(pos.x - directionX, pos.y + directionY))
				neighbours.push_back(m_pGraph->GetNode(int(pos.x - directionX), int(pos.y + directionY)));
		}
		if (IsNodeBlocked(pos.x, pos.y - directionY))
		{
			if (!IsNodeBlocked(pos.x + directionX, pos.y - directionY))
				neighbours.push_back(m_pGraph->GetNode(int(pos.x + directionX), int(pos.y - directionY)));
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

			if (IsNodeBlocked(nextPos.x - horizontal, nextPos.y))
			{
				if (!IsNodeBlocked(nextPos.x - horizontal, nextPos.y + vertical))
					return m_pGraph->GetNode(int(nextPos.x), int(nextPos.y));
			}

			if (IsNodeBlocked(nextPos.x, nextPos.y - vertical))
			{
				if (!IsNodeBlocked(nextPos.x + horizontal, nextPos.y - vertical))
					return m_pGraph->GetNode(int(nextPos.x), int(nextPos.y));
			}


			if (Jump(m_pGraph->GetNode(int(nextPos.x), int(nextPos.y)), horizontal, 0, pStartNode, pEndNode) != nullptr ||
				Jump(m_pGraph->GetNode(int(nextPos.x), int(nextPos.y)), 0, vertical, pStartNode, pEndNode) != nullptr)
			{
				return m_pGraph->GetNode(int(nextPos.x), int(nextPos.y));
			}


		}
		else if (horizontal != 0)//horizontal search
		{

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

		T_NodeType* nextNode{ m_pGraph->GetNode(int(nextPos.x), int(nextPos.y)) };
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

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> Elite::JPS<T_NodeType, T_ConnectionType>::GetJumpPoints() const
	{
		return m_JumpPoints;
	}


}