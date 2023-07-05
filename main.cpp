#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>

using namespace sf;

const float G = 0.1f; // Gravitational constant
const float theta = 0.5f; // Barnes-Hut opening angle

struct Node {
    Vector2f position;
    Vector2f velocity;
    Vector2f force;
    CircleShape shape;
    Color color;
};

struct Edge {
    Node* source;
    Node* target;
    VertexArray line;
    Color color;
};

Vector2f calculateForce(const Node& a, const Node& b) {
    Vector2f direction = b.position - a.position;
    float distance = sqrt(pow(direction.x, 2) + pow(direction.y, 2));

    if (distance != 0) {
        direction = direction / distance; // Normalize direction
    }

    float forceMagnitude = (G * a.shape.getRadius() * b.shape.getRadius()) / pow(distance, 2);
    return direction * forceMagnitude;
}

void applyForces(Node& node, float timeStep) {
    node.velocity += node.force * timeStep;
    node.position += node.velocity * timeStep;

    // Reset force
    node.force = Vector2f(0, 0);

    // Update the graphical representation of the node
    node.shape.setPosition(node.position);
}

void updateForces(Node& node, std::vector<Node>& nodes) {
    for (auto& otherNode : nodes) {
        if (&node != &otherNode) {
            node.force += calculateForce(node, otherNode);
        }
    }
}

void applyBarnesHut(Node& node, std::vector<Node>& nodes, Vector2f center, float size) {
    if (size / sqrt(pow(center.x - node.position.x, 2) + pow(center.y - node.position.y, 2)) < theta) {
        node.force += calculateForce(node, nodes[0]);
    } else {
        Vector2f centerOfMass(0.f, 0.f);
        float totalMass = 0.f;

        for (auto& otherNode : nodes) {
            if (&node != &otherNode) {
                centerOfMass += otherNode.shape.getRadius() * otherNode.position;
                totalMass += otherNode.shape.getRadius();
            }
        }

        centerOfMass /= totalMass;
        node.force += calculateForce(node, { .position = centerOfMass, .shape = CircleShape(size) });
    }
}

bool overlap(const Node& a, const Node& b) {
    float distance = sqrt(pow(a.position.x - b.position.x, 2) + pow(a.position.y - b.position.y, 2));
    return distance < a.shape.getRadius() + b.shape.getRadius();
}

void resolveOverlap(Node& a, Node& b) {
    Vector2f direction = a.position - b.position;
    float distance = sqrt(pow(direction.x, 2) + pow(direction.y, 2));
    float overlapAmount = (a.shape.getRadius() + b.shape.getRadius() - distance) / 2.f;
    direction /= distance;
    a.position += direction * overlapAmount;
    b.position -= direction * overlapAmount;
    a.shape.setPosition(a.position);
    b.shape.setPosition(b.position);
}

Color getRandomColor() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 255);
    return Color(dis(gen), dis(gen), dis(gen));
}

int main() {
    std::vector<Node> nodes(50);
    std::vector<Edge> edges;

    for (auto& node : nodes) {
        node.position = Vector2f(rand() % 800, rand() % 600);
        node.velocity = Vector2f(0.f, 0.f);
        node.force = Vector2f(0.f, 0.f);
        node.shape = CircleShape(10);
        node.color = getRandomColor();
        node.shape.setFillColor(node.color);
        node.shape.setPosition(node.position);
    }

    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = i + 1; j < nodes.size(); ++j) {
            edges.push_back({ &nodes[i], &nodes[j], VertexArray(Lines, 2), getRandomColor() });
        }
    }

    RenderWindow window(VideoMode(800, 600), "Network Visualization");

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        window.clear();

        for (auto& node : nodes) {
            updateForces(node, nodes);
        }

        for (auto& node : nodes) {
            applyBarnesHut(node, nodes, window.getView().getCenter(), window.getView().getSize().x);
        }

        for (auto& node : nodes) {
            applyForces(node, 0.1f);
        }

        for (auto& node : nodes) {
            for (auto& otherNode : nodes) {
                if (&node != &otherNode && overlap(node, otherNode)) {
                    resolveOverlap(node, otherNode);
                }
            }
        }

        for (auto& edge : edges) {
            edge.line[0].position = edge.source->position;
            edge.line[1].position = edge.target->position;
            edge.line[0].color = edge.color;
            edge.line[1].color = edge.color;
            window.draw(edge.line);
        }

        for (auto& node : nodes) {
            window.draw(node.shape);
        }

        window.display();
    }

    return 0;
}
