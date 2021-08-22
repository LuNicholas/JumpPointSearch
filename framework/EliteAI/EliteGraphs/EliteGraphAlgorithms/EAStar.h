#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		vector<T_NodeType*> path;
		vector<NodeRecord> openList;
		vector<NodeRecord> closedList;
		NodeRecord currentRecord{};

		NodeRecord startRecord{};
		startRecord.pNode = pStartNode;
		startRecord.pConnection = nullptr;
		startRecord.costSoFar = 0;
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);
		openList.push_back(startRecord);

		currentRecord = startRecord;

		while (!openList.empty())
		{
			

			auto bestRecord = std::min_element(openList.begin(), openList.end());
			currentRecord = *bestRecord;

			if (currentRecord.pNode == pGoalNode)
			{
				break;
			}


			for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				
				T_NodeType* nextNode = m_pGraph->GetNode(connection->GetTo());
				float gCost{ currentRecord.costSoFar + connection->GetCost() };	
				

				auto closedIt = std::find_if(closedList.begin(), closedList.end(), [nextNode](NodeRecord lhs) {return lhs.pNode == nextNode; });
				if (closedIt != closedList.end())
				{
					if (gCost >= closedIt->costSoFar)
					{
						continue;
					}
					else
					{
						closedList.erase(std::remove(closedList.begin(), closedList.end(), *closedIt));
					}
				}

				auto openIt = std::find_if(openList.begin(), openList.end(), [nextNode](NodeRecord lhs) {return lhs.pNode == nextNode; });
				if (openIt != openList.end())
				{
					if (gCost >= openIt->costSoFar)
					{
						continue;
					}
					else
					{
						openList.erase(std::remove(openList.begin(), openList.end(), *openIt));
					}
				}

				openList.push_back(NodeRecord{ nextNode, connection, gCost, gCost + GetHeuristicCost(nextNode, pGoalNode) });
			}

			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
			closedList.push_back(currentRecord);
		}

		while (currentRecord.pNode != startRecord.pNode)
		{
			path.push_back(currentRecord.pNode);

			for (const NodeRecord& closed : closedList)
			{
				if (closed.pNode->GetIndex() == currentRecord.pConnection->GetFrom())
				{
					currentRecord = closed;
					break;
				}
			}
		}
		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());


		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}