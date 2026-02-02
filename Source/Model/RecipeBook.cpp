#include "RecipeBook.h"

void RecipeBook::loadFromJSON (const juce::String& jsonString)
{
    recipes.clear();

    auto parsed = juce::JSON::parse (jsonString);
    if (auto* obj = parsed.getDynamicObject())
    {
        auto recipesArray = obj->getProperty ("recipes");
        if (auto* arr = recipesArray.getArray())
        {
            for (const auto& item : *arr)
            {
                if (auto* recipeObj = item.getDynamicObject())
                {
                    Recipe r;
                    r.ingredientA = { recipeObj->getProperty ("ingredientA").toString() };
                    r.ingredientB = { recipeObj->getProperty ("ingredientB").toString() };

                    auto result = recipeObj->getProperty ("result");
                    if (auto* resultObj = result.getDynamicObject())
                    {
                        r.resultId    = { resultObj->getProperty ("id").toString() };
                        r.resultName  = resultObj->getProperty ("name").toString();
                        r.samplePath  = resultObj->getProperty ("samplePath").toString();
                        r.visualKind  = parseVisualKind (resultObj->getProperty ("visualKind").toString());
                        r.colour      = juce::Colour::fromString (resultObj->getProperty ("colour").toString());

                        auto vp = resultObj->getProperty ("visualParams");
                        if (auto* vpObj = vp.getDynamicObject())
                        {
                            for (const auto& prop : vpObj->getProperties())
                                r.visualParams.set (prop.name.toString(), prop.value.toString());
                        }
                    }

                    recipes.push_back (std::move (r));
                }
            }
        }
    }
}

const Recipe* RecipeBook::findRecipe (const ElementID& a, const ElementID& b) const
{
    for (const auto& recipe : recipes)
        if (recipe.matches (a, b))
            return &recipe;
    return nullptr;
}

VisualElement::VisualKind RecipeBook::parseVisualKind (const juce::String& s)
{
    if (s.equalsIgnoreCase ("Geometric")) return VisualElement::VisualKind::Geometric;
    if (s.equalsIgnoreCase ("Particle"))  return VisualElement::VisualKind::Particle;
    return VisualElement::VisualKind::Waveform;
}
