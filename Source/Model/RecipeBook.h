#pragma once
#include "Recipe.h"
#include <vector>

class RecipeBook
{
public:
    RecipeBook() = default;

    void loadFromJSON (const juce::String& jsonString);
    const Recipe* findRecipe (const ElementID& a, const ElementID& b) const;
    const std::vector<Recipe>& getAllRecipes() const { return recipes; }

private:
    std::vector<Recipe> recipes;

    static VisualElement::VisualKind parseVisualKind (const juce::String& s);
};
