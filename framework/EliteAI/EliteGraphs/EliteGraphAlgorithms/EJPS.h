#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class JPS
	{
	public:
		JPS(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

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

		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		T_NodeType* Jump(T_NodeType* Parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		//T_NodeType HorSearch(T_NodeType* Parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		//T_NodeType VerSearch(T_NodeType* Parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	JPS<T_NodeType, T_ConnectionType>::JPS(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> JPS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		Jump(pStartNode, -1, 0, pStartNode, pGoalNode);

		return vector<T_NodeType*>();
	}

	template <class T_NodeType, class T_ConnectionType>
	T_NodeType* Elite::JPS<T_NodeType, T_ConnectionType>::Jump(T_NodeType* parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		//position of new node
		int connections{ 0 };

		T_NodeType* pNewNode = nullptr;
		for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(parent->GetIndex()))
		{
			connections++;
			if (m_pGraph->GetNodePos(connection->GetTo()) == Vector2{ m_pGraph->GetNodePos(parent).x + horizontal,  m_pGraph->GetNodePos(parent).y + vertical })
			{
				pNewNode = m_pGraph->GetNode(connection->GetTo());
				break;
			}
		}
		//all connections were wrong
		if (connections == m_pGraph->GetNodeConnections(parent->GetIndex()).size())
			return NULL;//there is no connection to the next node wanted


		//the new node found is the endNode
		if (pNewNode == pEndNode)
			return pNewNode;

		


	}



	template <class T_NodeType, class T_ConnectionType>
	float Elite::JPS<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}



}