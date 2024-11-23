// SFMLSerialization.hpp
#pragma once

#include <SFML/Graphics.hpp>
#include <boost/serialization/access.hpp>

namespace boost {
    namespace serialization {

        // Serialization for sf::Vector2f
        template<class Archive>
        void serialize(Archive& ar, sf::Vector2f& vec, const unsigned int version) {
            ar& vec.x;
            ar& vec.y;
        }

        // Serialization for sf::Color
        template<class Archive>
        void serialize(Archive& ar, sf::Color& color, const unsigned int version) {
            ar& color.r;
            ar& color.g;
            ar& color.b;
            ar& color.a;
        }

    } // namespace serialization
} // namespace boost
