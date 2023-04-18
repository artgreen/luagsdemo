-- Load the collection library
c = require("collection")

-- Create a new collection object
nums = c.newCollection(10)

-- Set some values in the collection
c.setCollectionValue(nums, 0, 42)
c.setCollectionValue(nums, 1, 23)
c.setCollectionValue(nums, 2, 12)
c.setCollectionValue(nums, 3, 99)

-- Get the size of the collection
size = c.getCollectionSize(nums)
print("Collection size:     ", size)

-- Get some values from the collection
print("Values in nums:")
for i = 0, size - 1 do
    value = c.getCollectionValue(nums, i)
    print(i, "     ", value)
end

-- Free the collection
c.freeCollection(nums)

