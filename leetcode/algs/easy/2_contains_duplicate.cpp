/*
Given an integer array nums, return true if any value appears more than once in the array, otherwise return false.

Example 1:
Input: nums = [1, 2, 3, 3]
Output: true

Example 2:
Input: nums = [1, 2, 3, 4]
Output: false

You should aim for a solution with O(n) time and O(n) space, where n is the size of the input array.
https://neetcode.io/problems/duplicate-integer
*/

class Solution {
public:
    bool hasDuplicate(vector<int>& nums) {
        if (nums.empty() == true)
            return 0;
        std::sort(nums.begin(), nums.end());
        int n = nums.size();
        for (int i = 0; i < n; ++i)
            if (nums[i] == nums[i + 1])
                return 1;
        return 0;
    }
};

// here we have some vector of integers. In my view we should sort that (std::sort - O(N*logN)) 
// and compare for equivalence => time complaxity is O(n logn) and space complexity is O(n) or O(1)

//second method O(n) and O(n) with unordered_map

class Solution2 {
public:
   bool hasDuplicate(vector<int>& nums) {
        unordered_set<int> us_;
        for (int _ : nums) 
        { if (us_.count(_)) {
                return true;
            }
            us_.insert(_);
        }
        return false; 
    } };

//third method: O(n) O(n) with the difference between set and vector

class Solution3 {
public:
    bool hasDuplicate(vector<int>& nums) {
        return unordered_set<int>(nums.begin(), nums.end()).size() < nums.size();
    } };
