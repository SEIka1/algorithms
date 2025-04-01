/*
Given an integer array nums and an integer k, return the k most frequent elements within the array.

The test cases are generated such that the answer is always unique.

You may return the output in any order.

Example 1:

Input: nums = [1,2,2,3,3,3], k = 2

Output: [2,3]
Example 2:

Input: nums = [7,7], k = 1

Output: [7]
Constraints:

1 <= nums.length <= 10^4.
-1000 <= nums[i] <= 1000
1 <= k <= number of distinct elements in nums.

https://neetcode.io/problems/top-k-elements-in-list

*/


//The first thing that came to mind was to do something like this:

class Solution2 {
public:
    vector<int> topKFrequent(vector<int>& nums, int k) {
        int num_items{0};
        int target{0};
        vector<int> vec;
        for (int i = 0; ((i < nums.size()) && (k != 0)); ++i)
        {
            if (num_items < std::count(nums.cbegin(), nums.cend(), nums[i])){
                num_items = std::count(nums.cbegin(), nums.cend(), nums[i]);
                target = nums[i];
            }
        }

        vec.push_back(target);
        return vec;
    }
};

/*
This code only works for k=1
Of course, you can just do it all in another "for" for k and here is the finished algorithm in O(n^2)
But the best you can do is O(n) for time complexity and O(n) for space complexity.
*/

class Solution {
public:
    vector<int> topKFrequent(vector<int>& nums, int k) {
        unordered_map<int, int> count;
        vector<vector<int>> freq(nums.size() + 1);

        for (int n : nums) {
            count[n] = 1 + count[n];
        }
        for (const auto& entry : count) {
            freq[entry.second].push_back(entry.first);
        }

        vector<int> res;
        for (int i = freq.size() - 1; i > 0; --i) {
            for (int n : freq[i]) {
                res.push_back(n);
                if (res.size() == k) {
                    return res;
                }
            }
        }
        return res;
    }
};

//here O(n) and O(n), so that you know!)
// it is not my solve of this problem, but a compromise between them. (actually, this is the best solve btw)
