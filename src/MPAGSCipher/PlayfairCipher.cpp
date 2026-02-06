#include "PlayfairCipher.hpp"

#include <algorithm>
#include <string>

/**
 * \file PlayfairCipher.cpp
 * \brief Contains the implementation of the PlayfairCipher class
 */

PlayfairCipher::PlayfairCipher(const std::string& key)
{
    this->setKey(key);
}

void PlayfairCipher::setKey(const std::string& key)
{
    // Store the original key
    key_ = key;

    // Append the alphabet to the key
    key_ += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // Make sure the key is upper case
    std::transform(std::begin(key_), std::end(key_), std::begin(key_),
                   ::toupper);

    // Remove non-alphabet characters
    key_.erase(std::remove_if(std::begin(key_), std::end(key_),
                              [](char c) { return !std::isalpha(c); }),
               std::end(key_));

    // Change J -> I
    std::transform(std::begin(key_), std::end(key_), std::begin(key_),
                   [](char c) { return (c == 'J') ? 'I' : c; });

    // Remove duplicated letters
    std::string lettersFound{""};
    auto detectDuplicates = [&](char c) {
        if (lettersFound.find(c) == std::string::npos) {
            lettersFound += c;
            return false;
        } else {
            return true;
        }
    };
    key_.erase(
        std::remove_if(std::begin(key_), std::end(key_), detectDuplicates),
        std::end(key_));

    // Store the coordinates of each letter
    // (at this point the key length must be equal to the square of the grid dimension)
    for (std::size_t i{0}; i < keyLength_; ++i) {
        std::size_t row{i / gridSize_};
        std::size_t column{i % gridSize_};

        auto coords = std::make_pair(row, column);

        charLookup_[key_[i]] = coords;
        coordLookup_[coords] = key_[i];
    }
}

std::string PlayfairCipher::applyCipher(const std::string& inputText,
                                        const CipherMode cipherMode) const
{
    // Create the output string, initially a copy of the input text
    std::string outputText{inputText};

    // Change J -> I
    std::transform(std::begin(outputText), std::end(outputText), std::begin(outputText),
                   [](char c) { return (c == 'J') ? 'I' : c; });

    // Find repeated characters and add an X (or a Q for repeated X's)
    for (std::size_t i{0}; i < outputText.size() - 1; ++i){
        if (outputText[i] == outputText[i+1]){
            if (outputText[i] == 'X'){
                outputText.insert(i+1, 1,'Q');
            }
            else{
                outputText.insert(i+1, 1,'X');
            }         
        }
    }

    // If the size of the input is odd, add a trailing Z
    if (outputText.size()%2 == 1){
        outputText.push_back('Z');
    }

    // Loop over the input bigrams
    for (std::size_t i{0}; i<outputText.size(); i+=2){

        // - Find the coordinates in the grid for each bigram
        PlayfairCoords firstPoint = charLookup_.at(outputText[i]);
        PlayfairCoords secondPoint = charLookup_.at(outputText[i+1]);
        auto& [xpos1, ypos1]{firstPoint};
        auto& [xpos2, ypos2]{secondPoint};
        
        // - Apply the rules to these coords to get new coords
        // same column
        if (xpos1 == xpos2){
            if (cipherMode == CipherMode::Encrypt){
                ypos1 = (ypos1 + 1) % gridSize_;
                ypos2 = (ypos2 + 1) % gridSize_;
            }
            else if (cipherMode == CipherMode::Decrypt){
                ypos1 = (ypos1 - 1) % gridSize_;
                ypos2 = (ypos2 - 1) % gridSize_;
            }
            
        }
        // same row
        else if (ypos1 == ypos2){
            if (cipherMode == CipherMode::Encrypt){
                xpos1 = (xpos1 + 1) % gridSize_;
                xpos2 = (xpos2 + 1) % gridSize_;
            }
            else if (cipherMode == CipherMode::Decrypt){
                xpos1 = (xpos1 - 1) % gridSize_;
                xpos2 = (xpos2 - 1) % gridSize_;
            }
        }
        // boxxing
        else {
            auto tmp{xpos1};
            xpos1 = xpos2;
            xpos2 = tmp;
        }
        // - Find the letters associated with the new coords
        char first = coordLookup_.at(firstPoint);
        char second = coordLookup_.at(secondPoint);

        // - Make the replacements
        outputText[i] = first;
        outputText[i+1] = second;
        }

    // Return the output text
    return outputText;
}
